#!/usr/bin/env python3
"""Rejects assert() calls whose argument list has a top-level comma.

`assert` is a function-like macro, and the preprocessor splits its arguments at every
comma that is not inside parentheses. Braces and angle brackets do not protect a comma,
so `assert(pair{1, 2} == pair{1, 2})` is two arguments. Some C libraries accept that
(their assert is variadic) and others reject it, so the same file can build on one
machine and fail on another. This check applies the preprocessor's rule everywhere.

Comments and string and character literals are skipped. Usage: CheckAssertCalls.py DIR...
"""

import re
import sys
from pathlib import Path

sys.dont_write_bytecode = True

CALL_RE = re.compile(r"(?<![A-Za-z0-9_])assert\s*\(")


def mask_literals(text):
    """Returns the text with comments and literal contents replaced by spaces."""
    out = list(text)
    i = 0
    n = len(text)
    while i < n:
        if text.startswith("//", i):
            end = text.find("\n", i)
            end = n if end == -1 else end
            out[i:end] = " " * (end - i)
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            end = n if end == -1 else end + 2
            for k in range(i, end):
                if out[k] != "\n":
                    out[k] = " "
            i = end
        elif text[i] in "\"'":
            quote = text[i]
            j = i + 1
            while j < n and text[j] != quote and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            for k in range(i + 1, min(j, n)):
                out[k] = " "
            i = j + 1
        else:
            i += 1
    return "".join(out)


def top_level_commas(masked, start):
    """Counts the commas inside the call whose '(' is at `start`, outside nested parentheses.

    Returns (count, line_offset), or None when the call is not closed.
    """
    depth = 0
    commas = 0
    for i in range(start, len(masked)):
        ch = masked[i]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return commas, i
        elif ch == "," and depth == 1:
            commas += 1
    return None


def check_file(path):
    text = path.read_text(encoding="utf-8", errors="replace")
    masked = mask_literals(text)
    problems = []

    for match in CALL_RE.finditer(masked):
        open_paren = masked.index("(", match.start())
        found = top_level_commas(masked, open_paren)

        if found is not None and found[0] > 0:
            line = masked.count("\n", 0, match.start()) + 1
            problems.append(f"{path}:{line}: assert() has {found[0] + 1} arguments; wrap the expression in parentheses")

    return problems


def main(argv):
    if len(argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    problems = []
    for root in argv[1:]:
        for path in sorted(Path(root).rglob("*.cpp")) + sorted(Path(root).rglob("*.hpp")):
            problems.extend(check_file(path))

    for problem in problems:
        print(problem)

    if problems:
        print(f"{len(problems)} assert() call(s) with a top-level comma", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
