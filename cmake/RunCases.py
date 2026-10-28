#!/usr/bin/env python3
"""Runs one example program against its `.cases` file.

Each case feeds `Input` to the program on stdin, then checks the exit code and
stdout against the `Output` alternatives. The syntax follows Moodle VPL's
`vpl_evaluate.cases`, reduced to the statements the examples use:

    Case = Title shown in the report
    Fail message = Shown instead of the default details on failure
    Input = text fed to stdin (may be empty)
    Output = "exact text"        exact stdout match
    Output = /pattern/flags      regular expression search (flags: i m s x)
    Output = 1 2.5 3             numbers only: stdout numbers in order
    Output = some words          loose text: letters and digits only, any case
    Program arguments = -a 1     argv for this case (shell quoting applies)
    Expected exit code = 0       defaults to 0

Statement names are case-insensitive. A value may continue on the following
lines until the next statement. A line that starts in column 0 with `word =`
must be a known statement, so indent any value line that would look like one.
Lines starting with `#` between statements are comments.

Usage:
    RunCases.py --program PATH --cases FILE --workdir DIR [--fixtures DIR]
                [--timeout SECONDS]
"""

import argparse
import math
import re
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

STATEMENT_RE = re.compile(
    r"^\s*(case|input|output|fail message|program arguments|expected exit code)\s*=(.*)$",
    re.IGNORECASE,
)
UNKNOWN_STATEMENT_RE = re.compile(r"^[A-Za-z][A-Za-z ]{0,30}\s*=")
REGEX_RE = re.compile(r"^/(.*)/([imsx]*)$", re.DOTALL)
NUMBER_RE = re.compile(r"^[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?$")
NUMBER_SEARCH_RE = re.compile(r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?")
FIXTURE_SKIP_SUFFIXES = {".cpp", ".cases", ".h", ".hpp", ".md"}
REGEX_FLAGS = {"i": re.IGNORECASE, "m": re.MULTILINE, "s": re.DOTALL, "x": re.VERBOSE}


class CasesError(Exception):
    """A `.cases` file that cannot be interpreted."""


@dataclass
class Expectation:
    """One `Output` alternative. `mode` is exact, regex, numeric or loose."""

    mode: str
    text: str
    pattern: Optional[re.Pattern] = None
    numbers: list[str] = field(default_factory=list)


@dataclass
class Case:
    line: int
    title: str = ""
    has_input: bool = False
    input: str = ""
    outputs: list[Expectation] = field(default_factory=list)
    fail_message: Optional[str] = None
    arguments: list[str] = field(default_factory=list)
    exit_code: int = 0


def normalize_newlines(text: str) -> str:
    return text.replace("\r\n", "\n").rstrip("\n")


def loose_key(text: str) -> str:
    return re.sub(r"[\W_]+", "", text).lower()


def build_expectation(value: str, line: int, source: str) -> Expectation:
    if len(value) >= 2 and value.startswith('"') and value.endswith('"'):
        return Expectation("exact", normalize_newlines(value[1:-1]))

    regex = REGEX_RE.match(value)
    if regex:
        flags = 0
        for flag in regex.group(2):
            flags |= REGEX_FLAGS[flag]
        try:
            pattern = re.compile(regex.group(1), flags)
        except re.error as error:
            raise CasesError(f"{source}:{line}: invalid regular expression: {error}")
        return Expectation("regex", value, pattern=pattern)

    tokens = value.split()
    if tokens and all(NUMBER_RE.match(token) for token in tokens):
        return Expectation("numeric", value, numbers=tokens)

    return Expectation("loose", value)


def parse_cases(text: str, source: str) -> list[Case]:
    cases: list[Case] = []
    current: Optional[Case] = None
    open_name: Optional[str] = None
    open_line = 0
    open_lines: list[str] = []

    def flush() -> None:
        nonlocal open_name, open_lines
        if open_name is None or current is None:
            return
        lines = list(open_lines)
        while lines and not lines[0].strip():
            lines.pop(0)
        value = "\n".join(lines).rstrip()
        apply_statement(current, open_name, value, open_line, source)
        open_name = None
        open_lines = []

    for number, raw in enumerate(text.splitlines(), start=1):
        match = STATEMENT_RE.match(raw)
        if match:
            flush()
            name = " ".join(match.group(1).lower().split())
            if name == "case":
                current = Case(line=number)
                cases.append(current)
            elif current is None:
                raise CasesError(f"{source}:{number}: '{name}' before the first Case")
            open_name = name
            open_line = number
            open_lines = [match.group(2).strip()]
            continue

        if open_name is not None and UNKNOWN_STATEMENT_RE.match(raw):
            word = raw.split("=", 1)[0].strip()
            raise CasesError(
                f"{source}:{number}: unknown statement '{word}' "
                "(a value line cannot start with 'word ='; reword it or quote the value)"
            )

        if open_name is not None:
            open_lines.append(raw)
        elif raw.strip() and not raw.lstrip().startswith("#"):
            raise CasesError(f"{source}:{number}: text outside a statement: {raw.strip()!r}")

    flush()

    for case in cases:
        if not case.title:
            raise CasesError(f"{source}:{case.line}: case has no title")
        if not case.has_input:
            raise CasesError(f"{source}:{case.line}: case has no Input statement")
        if not case.outputs:
            raise CasesError(f"{source}:{case.line}: case has no Output statement")
    if not cases:
        raise CasesError(f"{source}:1: no cases defined")
    return cases


def apply_statement(case: Case, name: str, value: str, line: int, source: str) -> None:
    if name == "case":
        case.title = value
    elif name == "input":
        case.has_input = True
        case.input = value + "\n" if value else ""
    elif name == "output":
        case.outputs.append(build_expectation(value, line, source))
    elif name == "fail message":
        case.fail_message = value
    elif name == "program arguments":
        try:
            case.arguments = shlex.split(value)
        except ValueError as error:
            raise CasesError(f"{source}:{line}: bad Program arguments: {error}")
    elif name == "expected exit code":
        try:
            case.exit_code = int(value)
        except ValueError:
            raise CasesError(f"{source}:{line}: Expected exit code must be an integer")


def matches(expectation: Expectation, actual: str) -> bool:
    if expectation.mode == "exact":
        return expectation.text == actual
    if expectation.mode == "regex":
        return expectation.pattern.search(actual) is not None
    if expectation.mode == "numeric":
        found = NUMBER_SEARCH_RE.findall(actual)
        if len(found) != len(expectation.numbers):
            return False
        for got, want in zip(found, expectation.numbers):
            got_value, want_value = float(got), float(want)
            if NUMBER_RE.match(want) and any(c in want for c in ".eE"):
                if not math.isclose(got_value, want_value, rel_tol=1e-6, abs_tol=1e-9):
                    return False
            elif got_value != want_value:
                return False
        return True
    return loose_key(expectation.text) == loose_key(actual)


def describe(expectations: list[Expectation]) -> str:
    return " | ".join(expectation.text for expectation in expectations)


def prepare_workdir(workdir: Path, fixtures: Optional[Path]) -> None:
    workdir = workdir.resolve()
    if "example_run" not in workdir.parts:
        raise SystemExit(f"refusing to clear {workdir}: it is not under an example_run directory")
    if workdir.exists():
        shutil.rmtree(workdir)
    workdir.mkdir(parents=True)
    if fixtures is not None and fixtures.is_dir():
        for entry in fixtures.iterdir():
            if entry.is_file() and entry.suffix not in FIXTURE_SKIP_SUFFIXES:
                shutil.copy2(entry, workdir / entry.name)


def run_case(program: Path, case: Case, workdir: Path, timeout: float) -> tuple[bool, str, str, str]:
    """Returns (passed, reason, stdout, stderr)."""
    try:
        completed = subprocess.run(
            [str(program), *case.arguments],
            input=case.input.encode("utf-8"),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=workdir,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return False, f"timed out after {timeout:g}s", "", ""

    stdout = normalize_newlines(completed.stdout.decode("utf-8", errors="replace"))
    stderr = completed.stderr.decode("utf-8", errors="replace")

    if completed.returncode != case.exit_code:
        return False, f"exit code {completed.returncode}, expected {case.exit_code}", stdout, stderr
    if not any(matches(expectation, stdout) for expectation in case.outputs):
        return False, "stdout did not match any Output", stdout, stderr
    return True, "", stdout, stderr


def indent(text: str, prefix: str = "    ", limit: int = 20) -> str:
    lines = text.splitlines()
    shown = [prefix + line for line in lines[:limit]]
    if len(lines) > limit:
        shown.append(f"{prefix}... ({len(lines) - limit} more lines)")
    return "\n".join(shown)


def report_failure(case: Case, reason: str, stdout: str, stderr: str) -> None:
    if case.fail_message is not None:
        print(f"  {case.fail_message}")
    else:
        print(f"  reason:   {reason}")
        print("  input:")
        print(indent(case.input.rstrip("\n") or "(empty)"))
        print(f"  expected: {describe(case.outputs)}")
        print("  actual:")
        print(indent(stdout or "(empty)"))
    if stderr.strip():
        print("  stderr:")
        print(indent(stderr.rstrip()))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--program", required=True, type=Path)
    parser.add_argument("--cases", required=True, type=Path)
    parser.add_argument("--workdir", required=True, type=Path)
    parser.add_argument("--fixtures", type=Path)
    parser.add_argument("--timeout", type=float, default=15.0)
    args = parser.parse_args()

    cases_path: Path = args.cases
    try:
        cases = parse_cases(cases_path.read_text(encoding="utf-8"), str(cases_path))
    except CasesError as error:
        print(f"error: {error}")
        return 1

    prepare_workdir(args.workdir, args.fixtures)
    program = args.program.resolve()
    workdir = args.workdir.resolve()

    passed = 0
    for index, case in enumerate(cases, start=1):
        ok, reason, stdout, stderr = run_case(program, case, workdir, args.timeout)
        label = f"Case {index}/{len(cases)}: {case.title}"
        if ok:
            passed += 1
            print(f"[PASS] {label}")
        else:
            print(f"[FAIL] {label}")
            report_failure(case, reason, stdout, stderr)

    print(f"{passed}/{len(cases)} cases passed")
    return 0 if passed == len(cases) else 1


if __name__ == "__main__":
    sys.exit(main())
