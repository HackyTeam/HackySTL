#!/usr/bin/env python3
"""Checks the layout and conventions of the Examples/ tree.

Every example `.cpp` needs a `// Level:` header and either a sibling `.cases`
file that parses, or a `// Manual:` header for programs that need a peer. The
examples print their results and never assert, so `assert(`, `<cassert>` and
`std::print` are rejected. Every `.cases` file needs a matching `.cpp`, and
every example must be listed in Examples/README.md under its target name.

Usage: CheckExamples.py EXAMPLES_DIR
"""

import re
import sys
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))

from RunCases import CasesError, parse_cases  # noqa: E402

LEVEL_RE = re.compile(r"^//\s*Level:\s*(Basic|Intermediate|Advanced)\s*$", re.MULTILINE)
MANUAL_RE = re.compile(r"^//\s*Manual:", re.MULTILINE)
BANNED = [
    (re.compile(r"(?<![A-Za-z0-9_])assert\s*\("), "uses assert(); examples check results by printing"),
    (re.compile(r"<cassert>"), "includes <cassert>"),
    (re.compile(r"\bstd::print"), "uses std::print; print with hsd::println instead"),
]


def target_name(relative: Path) -> str:
    return "hackystl_example_" + "_".join(relative.with_suffix("").parts)


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 2

    root = Path(sys.argv[1]).resolve()
    readme_path = root / "README.md"
    readme = readme_path.read_text(encoding="utf-8") if readme_path.exists() else ""
    errors: list[str] = []

    sources = sorted(root.rglob("*.cpp"))
    for source in sources:
        relative = source.relative_to(root)
        text = source.read_text(encoding="utf-8")
        header = "\n".join(text.splitlines()[:10])
        where = str(relative)

        if not LEVEL_RE.search(header):
            errors.append(f"{where}: missing '// Level: Basic|Intermediate|Advanced' in the first 10 lines")

        cases_path = source.with_suffix(".cases")
        if cases_path.exists():
            try:
                parse_cases(cases_path.read_text(encoding="utf-8"), str(cases_path.relative_to(root)))
            except CasesError as error:
                errors.append(str(error))
        elif not MANUAL_RE.search(header):
            errors.append(f"{where}: no .cases file and no '// Manual:' header explaining why")

        for pattern, message in BANNED:
            if pattern.search(text):
                errors.append(f"{where}: {message}")

        if readme and target_name(relative) not in readme:
            errors.append(f"{where}: target {target_name(relative)} is not listed in Examples/README.md")

    for cases_path in sorted(root.rglob("*.cases")):
        if not cases_path.with_suffix(".cpp").exists():
            errors.append(f"{cases_path.relative_to(root)}: no matching .cpp")

    for error in errors:
        print(f"error: {error}")
    print(f"checked {len(sources)} examples, {len(errors)} problem(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
