#!/usr/bin/env python3
"""Writes an HTML coverage report from gcovr JSON data.

The numbers match CoverageSites.py: each source line, branch and function counts once,
and is covered when any template instance runs it. Lines excluded with GCOVR_EXCL
markers are left out. The report has a summary page with a table per file, and one
page per source file with the source colored by coverage. A button switches between
light and dark themes; the theme follows the system setting until the button is used,
and the choice is remembered in the browser.

Usage: CoverageReport.py COVERAGE.json SOURCE_DIR OUTPUT_DIR
"""

import html
import json
import sys
from collections import defaultdict
from datetime import datetime
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))

from CoverageSites import percent, read_sites  # noqa: E402

THEME_KEY = "hsd-coverage-theme"

LIGHT_TOKENS = """
    --bg: #f5f6f8;
    --surface: #ffffff;
    --surface-alt: #f0f2f5;
    --text: #1b2430;
    --muted: #5d6878;
    --border: #d8dee7;
    --accent: #4338ca;
    --good: #15803d;
    --good-bg: #dcfce7;
    --warn: #b45309;
    --warn-bg: #fef3c7;
    --bad: #b91c1c;
    --bad-bg: #fee2e2;
    --excluded: #7b8492;
    --track: #e4e8ee;
    --shadow: 0 1px 2px rgba(16, 24, 40, 0.06), 0 6px 18px rgba(16, 24, 40, 0.06);
    color-scheme: light;
"""

DARK_TOKENS = """
    --bg: #0d1117;
    --surface: #161b22;
    --surface-alt: #1c2330;
    --text: #e6edf3;
    --muted: #9aa6b2;
    --border: #2d3644;
    --accent: #8b9dff;
    --good: #4ade80;
    --good-bg: rgba(74, 222, 128, 0.14);
    --warn: #fbbf24;
    --warn-bg: rgba(251, 191, 36, 0.16);
    --bad: #f87171;
    --bad-bg: rgba(248, 113, 113, 0.16);
    --excluded: #7d8896;
    --track: #2a3340;
    --shadow: 0 1px 2px rgba(0, 0, 0, 0.4), 0 8px 24px rgba(0, 0, 0, 0.35);
    color-scheme: dark;
"""

STYLE = f"""
:root {{
{LIGHT_TOKENS}
}}
@media (prefers-color-scheme: dark) {{
  :root:not([data-theme="light"]) {{
{DARK_TOKENS}
  }}
}}
:root[data-theme="dark"] {{
{DARK_TOKENS}
}}

* {{ box-sizing: border-box; }}
html, body {{ margin: 0; padding: 0; }}
body {{
  background: var(--bg);
  color: var(--text);
  font: 15px/1.5 system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
}}
a {{ color: var(--accent); text-decoration: none; }}
a:hover {{ text-decoration: underline; }}
code, .mono {{ font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, "Liberation Mono", monospace; }}

header.top {{
  position: sticky; top: 0; z-index: 10;
  display: flex; align-items: center; justify-content: space-between; gap: 16px;
  padding: 14px clamp(16px, 4vw, 40px);
  background: var(--surface);
  background: color-mix(in srgb, var(--surface) 88%, transparent);
  backdrop-filter: saturate(1.4) blur(10px);
  border-bottom: 1px solid var(--border);
}}
header.top h1 {{ font-size: 17px; margin: 0; font-weight: 650; letter-spacing: -0.01em; }}
header.top .meta {{ color: var(--muted); font-size: 13px; }}
.crumb {{ color: var(--muted); font-size: 13px; }}

button.theme {{
  display: inline-flex; align-items: center; gap: 8px;
  padding: 7px 12px; border-radius: 999px;
  border: 1px solid var(--border); background: var(--surface); color: var(--text);
  font: inherit; font-size: 13px; cursor: pointer; white-space: nowrap;
  box-shadow: var(--shadow);
}}
button.theme:hover {{ border-color: var(--accent); }}
button.theme:focus-visible, input.filter:focus-visible, th[role="button"]:focus-visible, a:focus-visible {{
  outline: 2px solid var(--accent); outline-offset: 2px;
}}
button.theme .glyph {{ font-size: 15px; line-height: 1; }}

main {{ max-width: 1180px; margin: 0 auto; padding: 28px clamp(16px, 4vw, 40px) 56px; }}
h2 {{ font-size: 15px; font-weight: 650; margin: 32px 0 12px; }}
p.note {{ color: var(--muted); font-size: 13px; margin: 8px 0 0; max-width: 78ch; }}

.cards {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 16px; }}
.card {{
  background: var(--surface); border: 1px solid var(--border); border-radius: 14px;
  padding: 18px 20px; box-shadow: var(--shadow);
}}
.card .label {{ color: var(--muted); font-size: 13px; }}
.card .figure {{ display: flex; align-items: baseline; gap: 10px; margin-top: 4px; }}
.card .percent {{ font-size: 32px; font-weight: 700; letter-spacing: -0.02em; font-variant-numeric: tabular-nums; }}
.card .counts {{ color: var(--muted); font-size: 13px; font-variant-numeric: tabular-nums; }}

.bar {{ height: 8px; border-radius: 999px; background: var(--track); overflow: hidden; margin-top: 12px; }}
.bar > span {{ display: block; height: 100%; border-radius: inherit; background: var(--accent); }}
.bar.good > span {{ background: var(--good); }}
.bar.warn > span {{ background: var(--warn); }}
.bar.bad > span {{ background: var(--bad); }}

.pill {{
  display: inline-block; min-width: 3.8em; padding: 1px 8px; border-radius: 999px;
  font-size: 12.5px; font-weight: 600; text-align: center; font-variant-numeric: tabular-nums;
}}
.pill.good {{ color: var(--good); background: var(--good-bg); }}
.pill.warn {{ color: var(--warn); background: var(--warn-bg); }}
.pill.bad {{ color: var(--bad); background: var(--bad-bg); }}
.pill.none {{ color: var(--muted); background: var(--surface-alt); }}

.toolbar {{ display: flex; align-items: center; justify-content: space-between; gap: 12px; flex-wrap: wrap; margin-top: 32px; }}
.toolbar h2 {{ margin: 0; }}
input.filter {{
  width: min(320px, 100%); padding: 8px 12px; border-radius: 10px;
  border: 1px solid var(--border); background: var(--surface); color: var(--text); font: inherit; font-size: 14px;
}}

.table-wrap {{
  margin-top: 12px; background: var(--surface); border: 1px solid var(--border);
  border-radius: 14px; box-shadow: var(--shadow); overflow-x: auto;
}}
table {{ border-collapse: collapse; width: 100%; }}
th, td {{ padding: 10px 14px; text-align: left; border-bottom: 1px solid var(--border); vertical-align: middle; }}
tr:last-child > td {{ border-bottom: 0; }}
th {{
  font-size: 12px; font-weight: 600; color: var(--muted); text-transform: uppercase; letter-spacing: 0.04em;
  background: var(--surface-alt); white-space: nowrap; user-select: none;
}}
th[role="button"] {{ cursor: pointer; }}
th[role="button"]::after {{ content: " \\2195"; opacity: 0.45; }}
th[aria-sort="ascending"]::after {{ content: " \\2191"; opacity: 1; }}
th[aria-sort="descending"]::after {{ content: " \\2193"; opacity: 1; }}
td.num {{ text-align: right; font-variant-numeric: tabular-nums; white-space: nowrap; }}
td.file a {{ font-weight: 550; }}
td.bar-cell {{ width: 28%; min-width: 140px; }}
td.bar-cell .bar {{ margin-top: 0; }}
tbody tr:hover {{ background: var(--surface-alt); }}

.source-meta {{ display: flex; gap: 12px; flex-wrap: wrap; margin: 18px 0 8px; }}
.source-meta .chip {{
  padding: 4px 10px; border-radius: 999px; background: var(--surface); border: 1px solid var(--border);
  font-size: 13px; color: var(--muted);
}}
.legend {{ display: flex; gap: 14px; flex-wrap: wrap; font-size: 13px; color: var(--muted); margin: 8px 0 14px; }}
.legend span::before {{
  content: ""; display: inline-block; width: 10px; height: 10px; border-radius: 3px; margin-right: 6px; vertical-align: -1px;
  background: var(--c);
}}
.uncovered-list {{ margin: 8px 0 0; padding-left: 18px; font-size: 13.5px; }}
.uncovered-list li {{ margin: 3px 0; }}

.source {{ font-size: 13px; }}
.source table {{ table-layout: fixed; }}
.source td {{ padding: 0 12px; border: 0; line-height: 20px; vertical-align: top; }}
.source td.ln {{
  width: 64px; text-align: right; color: var(--muted); user-select: none; font-variant-numeric: tabular-nums;
  border-right: 1px solid var(--border);
}}
.source td.hits {{ width: 88px; text-align: right; color: var(--muted); font-variant-numeric: tabular-nums; white-space: nowrap; }}
.source td.code {{ white-space: pre; overflow-x: visible; }}
.source tr.cov td.hits {{ color: var(--good); }}
.source tr.cov {{ background: var(--good-bg); }}
.source tr.part {{ background: var(--warn-bg); }}
.source tr.part td.hits {{ color: var(--warn); }}
.source tr.miss {{ background: var(--bad-bg); }}
.source tr.miss td.hits {{ color: var(--bad); font-weight: 600; }}
.source tr.excl td.code, .source tr.excl td.hits {{ color: var(--excluded); font-style: italic; }}
.source tr.cov td.ln, .source tr.part td.ln, .source tr.miss td.ln {{ border-right-color: transparent; }}

footer {{ color: var(--muted); font-size: 12.5px; margin-top: 40px; }}

@media (max-width: 640px) {{
  header.top .meta {{ display: none; }}
  th, td {{ padding: 9px 10px; }}
  .card .percent {{ font-size: 28px; }}
  .source td.hits {{ width: 56px; }}
  .source td.ln {{ width: 48px; }}
}}
"""

SCRIPT = f"""
(function () {{
  var key = "{THEME_KEY}";
  var root = document.documentElement;

  function systemTheme() {{
    return window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }}

  function currentTheme() {{
    return root.getAttribute("data-theme") || systemTheme();
  }}

  function paintButton() {{
    var button = document.getElementById("theme-toggle");
    if (!button) return;
    var dark = currentTheme() === "dark";
    button.setAttribute("aria-pressed", dark ? "true" : "false");
    button.querySelector(".label").textContent = dark ? "Light mode" : "Dark mode";
    button.querySelector(".glyph").textContent = dark ? "\\u2600" : "\\u263E";
  }}

  function setTheme(theme) {{
    root.setAttribute("data-theme", theme);
    try {{ localStorage.setItem(key, theme); }} catch (error) {{ /* storage may be blocked */ }}
    paintButton();
  }}

  document.addEventListener("DOMContentLoaded", function () {{
    var button = document.getElementById("theme-toggle");
    if (button) {{
      button.addEventListener("click", function () {{
        setTheme(currentTheme() === "dark" ? "light" : "dark");
      }});
    }}
    paintButton();

    var filter = document.getElementById("file-filter");
    if (filter) {{
      filter.addEventListener("input", function () {{
        var needle = filter.value.trim().toLowerCase();
        var rows = document.querySelectorAll("tbody.files tr");
        for (var i = 0; i < rows.length; i++) {{
          var name = rows[i].getAttribute("data-name");
          rows[i].hidden = needle !== "" && name.indexOf(needle) === -1;
        }}
      }});
    }}

    var headers = document.querySelectorAll("th[role='button']");
    for (var h = 0; h < headers.length; h++) {{
      headers[h].addEventListener("click", sortBy(headers[h]));
      headers[h].addEventListener("keydown", function (event) {{
        if (event.key === "Enter" || event.key === " ") {{
          event.preventDefault();
          this.click();
        }}
      }});
    }}
  }});

  function sortBy(header) {{
    return function () {{
      var table = header.closest("table");
      var body = table.tBodies[0];
      var index = Array.prototype.indexOf.call(header.parentNode.children, header);
      var descending = header.getAttribute("aria-sort") !== "descending";

      var all = table.querySelectorAll("th[role='button']");
      for (var i = 0; i < all.length; i++) all[i].removeAttribute("aria-sort");
      header.setAttribute("aria-sort", descending ? "descending" : "ascending");

      var rows = Array.prototype.slice.call(body.rows);
      rows.sort(function (a, b) {{
        var left = a.cells[index].getAttribute("data-value");
        var right = b.cells[index].getAttribute("data-value");
        var numeric = !isNaN(parseFloat(left)) && !isNaN(parseFloat(right));
        var result = numeric ? parseFloat(left) - parseFloat(right) : left.localeCompare(right);
        return descending ? -result : result;
      }});
      for (var r = 0; r < rows.length; r++) body.appendChild(rows[r]);
    }};
  }}
}})();
"""

HEAD_SCRIPT = (
    "<script>try{var t=localStorage.getItem('" + THEME_KEY + "');"
    "if(t==='light'||t==='dark'){document.documentElement.setAttribute('data-theme',t);}}"
    "catch(e){}</script>"
)

TOGGLE = (
    '<button type="button" class="theme" id="theme-toggle" aria-pressed="false">'
    '<span class="glyph" aria-hidden="true"></span><span class="label">Dark mode</span></button>'
)


def level(value):
    """Returns the CSS class for a percentage: good from 90, warn from 75, else bad."""
    if value >= 90.0:
        return "good"
    if value >= 75.0:
        return "warn"
    return "bad"


def fmt(value):
    return f"{value:.1f}%"


def meter(covered, total):
    """Returns a percentage pill and a bar for one measure."""
    value = percent(covered, total)
    css = level(value) if total else "none"
    text = "n/a" if total == 0 else fmt(value)
    bar_width = 100.0 if total == 0 else value
    return (
        f'<span class="pill {css}">{text}</span>',
        f'<div class="bar {css}" role="presentation"><span style="width:{bar_width:.1f}%"></span></div>',
    )


def page(title, body, crumb=None, base=""):
    crumb_html = f'<div class="crumb">{crumb}</div>' if crumb else ""
    return f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="color-scheme" content="light dark">
<title>{html.escape(title)}</title>
{HEAD_SCRIPT}
<style>{STYLE}</style>
</head>
<body>
<header class="top">
  <div>
    <h1>HackySTL coverage</h1>
    {crumb_html}
  </div>
  <div style="display:flex;align-items:center;gap:14px">
    <span class="meta">{html.escape(base)}</span>
    {TOGGLE}
  </div>
</header>
<main>
{body}
</main>
<script>{SCRIPT}</script>
</body>
</html>
"""


class Data:
    """Per-site counts, grouped by file, and per-line status."""

    def __init__(self, json_path):
        self.lines, self.branches, self.functions, self.names = read_sites(json_path)

        report = json.loads(Path(json_path).read_text(encoding="utf-8"))
        self.hits = {}
        self.excluded = set()
        for entry in report.get("files", []):
            file_name = entry["file"]
            for line in entry.get("lines", []):
                key = (file_name, line["line_number"])
                self.hits[key] = max(self.hits.get(key, 0), line.get("count", 0))
                if line.get("gcovr/excluded"):
                    self.excluded.add(key)

        self.files = sorted({key[0] for key in self.lines} | {key[0] for key in self.functions})
        self.missed_branches = defaultdict(list)
        for (file_name, line_number, index), covered in self.branches.items():
            if not covered:
                self.missed_branches[(file_name, line_number)].append(index)

    def file_totals(self, file_name):
        lines = [v for (f, _), v in self.lines.items() if f == file_name]
        branches = [v for (f, _, _), v in self.branches.items() if f == file_name]
        functions = [v for (f, _), v in self.functions.items() if f == file_name]
        return (
            (sum(lines), len(lines)),
            (sum(branches), len(branches)),
            (sum(functions), len(functions)),
        )

    def totals(self):
        def pair(sites):
            return (sum(sites.values()), len(sites))
        return pair(self.lines), pair(self.branches), pair(self.functions)

    def line_status(self, file_name, line_number):
        key = (file_name, line_number)
        if key in self.lines:
            if not self.lines[key]:
                return "miss"
            if self.missed_branches.get(key):
                return "part"
            return "cov"
        if key in self.excluded:
            return "excl"
        return ""


def file_page_name(file_name):
    return "files/" + file_name.replace("/", "__") + ".html"


def write_index(data, out_dir, generated):
    (lines, branches, functions) = data.totals()
    cards = []
    for label, (covered, total) in (("Lines", lines), ("Branches", branches), ("Functions", functions)):
        pill, bar = meter(covered, total)
        cards.append(
            f'<div class="card"><div class="label">{label}</div>'
            f'<div class="figure"><span class="percent">{fmt(percent(covered, total))}</span>{pill}</div>'
            f'<div class="counts">{covered:,} of {total:,} covered</div>{bar}</div>'
        )

    rows = []
    for file_name in data.files:
        (line_cov, line_total), (branch_cov, branch_total), (func_cov, func_total) = data.file_totals(file_name)
        cells = []
        for covered, total in ((line_cov, line_total), (branch_cov, branch_total), (func_cov, func_total)):
            value = percent(covered, total)
            cells.append(
                f'<td class="num" data-value="{value:.4f}">{meter(covered, total)[0]}'
                f'<span class="counts"> &nbsp;{covered:,}/{total:,}</span></td>'
            )
        bar_value = percent(line_cov, line_total)
        rows.append(
            f'<tr data-name="{html.escape(file_name.lower())}">'
            f'<td class="file" data-value="{html.escape(file_name)}">'
            f'<a href="{file_page_name(file_name)}">{html.escape(file_name)}</a></td>'
            + "".join(cells)
            + f'<td class="bar-cell" data-value="{bar_value:.4f}">'
            f'<div class="bar {level(bar_value)}"><span style="width:{bar_value:.1f}%"></span></div></td>'
            "</tr>"
        )

    header_cells = (
        '<th role="button" tabindex="0">File</th>'
        '<th role="button" tabindex="0">Lines</th>'
        '<th role="button" tabindex="0">Branches</th>'
        '<th role="button" tabindex="0">Functions</th>'
        '<th>Lines bar</th>'
    )

    body = f"""
<div class="cards">{''.join(cards)}</div>
<p class="note">Counted per source site: each line, branch and function counts once, and is covered
when any template instance runs it. Lines excluded with GCOVR markers are left out. Generated
{generated}.</p>

<div class="toolbar">
  <h2>Files</h2>
  <input class="filter" id="file-filter" type="search" placeholder="Filter files" aria-label="Filter files">
</div>
<div class="table-wrap">
<table>
  <thead><tr>{header_cells}</tr></thead>
  <tbody class="files">
    {''.join(rows)}
  </tbody>
</table>
</div>
<footer>Click a column heading to sort. Files and lines use the same thresholds as the badges:
90% and above is good, 75% and above is fair, and below 75% needs attention.</footer>
"""
    (out_dir / "index.html").write_text(
        page("HackySTL coverage", body, base=f"{generated}"), encoding="utf-8"
    )


def write_file_page(data, out_dir, file_name, source_dir):
    (line_cov, line_total), (branch_cov, branch_total), (func_cov, func_total) = data.file_totals(file_name)

    cards = []
    for label, covered, total in (
        ("Lines", line_cov, line_total),
        ("Branches", branch_cov, branch_total),
        ("Functions", func_cov, func_total),
    ):
        pill, bar = meter(covered, total)
        cards.append(
            f'<div class="card"><div class="label">{label}</div>'
            f'<div class="figure"><span class="percent">{fmt(percent(covered, total))}</span>{pill}</div>'
            f'<div class="counts">{covered:,} of {total:,}</div>{bar}</div>'
        )

    missed_functions = sorted(
        (line, data.names.get((file_name, line), "")) for (f, line), covered in data.functions.items()
        if f == file_name and not covered
    )
    if missed_functions:
        items = "".join(
            f'<li><a href="#L{line}">line {line}</a> <span class="mono">{html.escape(name[:160])}</span></li>'
            for line, name in missed_functions
        )
        functions_html = f'<h2>Functions not run</h2><ul class="uncovered-list">{items}</ul>'
    else:
        functions_html = ""

    source_path = Path(source_dir) / file_name
    if source_path.is_file():
        source_lines = source_path.read_text(encoding="utf-8", errors="replace").splitlines()
    else:
        source_lines = [f"(source not found: {file_name})"]

    rows = []
    for number, text in enumerate(source_lines, start=1):
        status = data.line_status(file_name, number)
        css = f' class="{status}"' if status else ""
        hits = ""
        title = ""
        if status == "excl":
            hits = "excluded"
        elif status:
            hits = str(data.hits.get((file_name, number), 0))
            if status == "part":
                missed = data.missed_branches.get((file_name, number), [])
                title = f' title="branch {", ".join(str(i) for i in missed)} not taken"'
        rows.append(
            f'<tr id="L{number}"{css}{title}><td class="ln">{number}</td>'
            f'<td class="hits">{hits}</td><td class="code">{html.escape(text)}</td></tr>'
        )

    legend = (
        '<div class="legend">'
        '<span style="--c: var(--good-bg)">Run</span>'
        '<span style="--c: var(--warn-bg)">Run, some branches not taken</span>'
        '<span style="--c: var(--bad-bg)">Not run</span>'
        '<span style="--c: var(--track)">Excluded or not code</span>'
        '</div>'
    )

    body = f"""
<div class="cards">{''.join(cards)}</div>
{functions_html}
<div class="source-meta"><span class="chip">{html.escape(file_name)}</span></div>
{legend}
<div class="table-wrap source">
<table>
  <tbody>
    {''.join(rows)}
  </tbody>
</table>
</div>
"""
    crumb = f'<a href="../index.html">Summary</a> &rsaquo; <span class="mono">{html.escape(file_name)}</span>'
    (out_dir / file_page_name(file_name)).write_text(
        page(f"{file_name} - HackySTL coverage", body, crumb=crumb, base=file_name), encoding="utf-8"
    )


def main(argv):
    if len(argv) != 4:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    json_path, source_dir, out_dir = argv[1], Path(argv[2]), Path(argv[3])
    data = Data(json_path)

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "files").mkdir(exist_ok=True)

    generated = datetime.now().strftime("%Y-%m-%d %H:%M")
    write_index(data, out_dir, generated)
    for file_name in data.files:
        write_file_page(data, out_dir, file_name, source_dir)

    print(f"Coverage report: {out_dir / 'index.html'}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
