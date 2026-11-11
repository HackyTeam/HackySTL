#!/usr/bin/env python3
"""Summarizes gcovr JSON per source site instead of per template instance.

gcovr counts every template instance of a line, branch and function separately, so
a template that is used with several types needs each type to run every line. This
script counts a source line, branch or function once, as covered when any instance
of it runs. Lines and branches excluded with GCOVR_EXCL markers are left out.

Usage:
    CoverageSites.py COVERAGE.json [--list] [--fail-under-lines N]
                     [--fail-under-functions N] [--fail-under-branches N]

Exit status is 1 when a threshold is not met, and 0 otherwise. Thresholds are
optional, so the script can print the summary without failing the build.
"""

import argparse
import collections
import json
import sys


def read_sites(path):
    """Returns the covered flags for each unique line, branch and function site."""
    with open(path, encoding="utf-8") as handle:
        report = json.load(handle)

    lines = {}
    branches = {}
    functions = {}
    names = {}

    for entry in report.get("files", []):
        file_name = entry["file"]

        for line in entry.get("lines", []):
            if line.get("gcovr/excluded"):
                continue

            key = (file_name, line["line_number"])
            lines[key] = lines.get(key, False) or line["count"] > 0

            for index, branch in enumerate(line.get("branches", [])):
                if branch.get("gcovr/excluded"):
                    continue

                branch_key = (file_name, line["line_number"], index)
                branches[branch_key] = branches.get(branch_key, False) or branch["count"] > 0

        for function in entry.get("functions", []):
            key = (file_name, function["lineno"])
            functions[key] = functions.get(key, False) or function["execution_count"] > 0
            names.setdefault(key, function.get("demangled_name", function.get("name", "")))

    return lines, branches, functions, names


def percent(covered, total):
    return 100.0 * covered / total if total else 100.0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("json", help="gcovr JSON report")
    parser.add_argument("--list", action="store_true",
                        help="print every uncovered line, branch and function site")
    parser.add_argument("--fail-under-lines", type=float, default=None)
    parser.add_argument("--fail-under-functions", type=float, default=None)
    parser.add_argument("--fail-under-branches", type=float, default=None)
    args = parser.parse_args()

    lines, branches, functions, names = read_sites(args.json)

    totals = {
        "lines": (sum(lines.values()), len(lines)),
        "branches": (sum(branches.values()), len(branches)),
        "functions": (sum(functions.values()), len(functions)),
    }

    print("Coverage per source site (each line, branch and function counted once):")
    for label in ("lines", "branches", "functions"):
        covered, total = totals[label]
        print(f"  {label:<10} {percent(covered, total):6.2f}%  ({covered} of {total})")

    if args.list:
        uncovered = collections.defaultdict(list)
        for (file_name, line_number), covered in lines.items():
            if not covered:
                uncovered[file_name].append(f"line {line_number}")
        for (file_name, line_number, index), covered in branches.items():
            if not covered:
                uncovered[file_name].append(f"branch {line_number}#{index}")
        for (file_name, line_number), covered in functions.items():
            if not covered:
                uncovered[file_name].append(f"function {line_number}: {names[(file_name, line_number)][:100]}")

        for file_name in sorted(uncovered):
            print(file_name)
            for item in sorted(uncovered[file_name]):
                print(f"    {item}")

    thresholds = (
        ("lines", args.fail_under_lines),
        ("branches", args.fail_under_branches),
        ("functions", args.fail_under_functions),
    )
    failed = False
    for label, minimum in thresholds:
        if minimum is None:
            continue
        covered, total = totals[label]
        if percent(covered, total) < minimum:
            print(f"Coverage of {label} is below {minimum}%", file=sys.stderr)
            failed = True

    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
