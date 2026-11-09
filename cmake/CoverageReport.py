#!/usr/bin/env python3
"""Writes an HTML coverage report from gcovr JSON data.

The numbers match CoverageSites.py: each source line, branch and function counts once,
and is covered when any template instance runs it. Lines excluded with GCOVR_EXCL
markers are left out. The report has a summary page with a table per file, and one
page per source file with the source highlighted as C++ and colored by coverage.

The colors are the Dracula palette for the dark theme and Alucard, Dracula's official
light theme, for the light one. The theme follows the system setting until the button
is used, and the choice is remembered in the browser.

Usage: CoverageReport.py COVERAGE.json SOURCE_DIR OUTPUT_DIR
"""

import html
import json
import re
import sys
from collections import defaultdict
from datetime import datetime
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))

from CoverageSites import percent, read_sites  # noqa: E402

THEME_KEY = "hsd-coverage-theme"

# Palettes from https://draculatheme.com/spec: Dracula Classic and Alucard Classic.
DRACULA = {
    "bg": "#282A36", "fg": "#F8F8F2", "comment": "#6272A4", "selection": "#44475A",
    "cyan": "#8BE9FD", "green": "#50FA7B", "orange": "#FFB86C", "pink": "#FF79C6",
    "purple": "#BD93F9", "red": "#FF5555", "yellow": "#F1FA8C",
}
ALUCARD = {
    "bg": "#FFFBEB", "fg": "#1F1F1F", "comment": "#6C664B", "selection": "#CFCFDE",
    "cyan": "#036A96", "green": "#14710A", "orange": "#A34D14", "pink": "#A3144D",
    "purple": "#644AC9", "red": "#CB3A2A", "yellow": "#846E15",
}

CPP_KEYWORDS = frozenset("""
    alignas alignof and and_eq asm auto bitand bitor bool break case catch char char8_t
    char16_t char32_t class compl concept const consteval constexpr constinit const_cast
    continue co_await co_return co_yield decltype default delete do double dynamic_cast else
    enum explicit export extern false final float for friend goto if inline int long mutable
    namespace new noexcept not not_eq nullptr operator or or_eq override private protected
    public register reinterpret_cast requires return short signed sizeof static static_assert
    static_cast struct switch template this thread_local throw true try typedef typeid
    typename union unsigned using virtual void volatile wchar_t while xor xor_eq
""".split())
CONSTANTS = frozenset({"true", "false", "nullptr"})
TYPE_INTRO = frozenset({"class", "struct", "union", "enum", "typename", "namespace", "using"})

TOKEN_RE = re.compile(
    r"(?P<comment>//.*|/\*.*?\*/|/\*.*)"
    r"|(?P<string>\"(?:\\.|[^\"\\])*\"?)"
    r"|(?P<char>'(?:\\.|[^'\\])*'?)"
    r"|(?P<number>(?:0[xX][0-9A-Fa-f'.]+(?:[pP][+-]?\d+)?|\d[\d']*(?:\.\d*)?(?:[eE][+-]?\d+)?)[uUlLfF]*)"
    r"|(?P<word>[A-Za-z_]\w*)"
    r"|(?P<space>\s+)"
    r"|(?P<other>.)"
)

STYLE = r"""
:root {
%%LIGHT%%
}
@media (prefers-color-scheme: dark) {
  :root:not([data-theme="light"]) {
%%DARK%%
  }
}
:root[data-theme="dark"] {
%%DARK%%
}

* { box-sizing: border-box; }
html, body { margin: 0; padding: 0; }
body {
  background: var(--bg);
  color: var(--text);
  font: 16px/1.55 system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
}
a { color: var(--accent); text-decoration: none; }
a:hover { text-decoration: underline; }
.mono, code {
  font-family: "JetBrains Mono", "Cascadia Code", ui-monospace, SFMono-Regular, Menlo, Consolas,
    "Liberation Mono", monospace;
}

header.top {
  position: sticky; top: 0; z-index: 10;
  display: flex; align-items: center; justify-content: space-between; gap: 16px;
  padding: 16px clamp(16px, 4vw, 40px);
  background: var(--surface);
  border-bottom: 1px solid var(--border);
}
header.top h1 { font-size: 20px; margin: 0; font-weight: 650; letter-spacing: -0.01em; }
header.top .meta { color: var(--muted); font-size: 15px; }
.crumb { color: var(--muted); font-size: 15px; margin-top: 2px; }

button.theme {
  display: inline-flex; align-items: center; gap: 8px;
  padding: 8px 14px; border-radius: 999px;
  border: 1px solid var(--border); background: var(--surface); color: var(--text);
  font: inherit; font-size: 15px; cursor: pointer; white-space: nowrap;
  box-shadow: var(--shadow);
}
button.theme:hover { border-color: var(--accent); }
button.theme:focus-visible, input.filter:focus-visible, th[role="button"]:focus-visible, a:focus-visible {
  outline: 2px solid var(--accent); outline-offset: 2px;
}
button.theme .glyph { font-size: 16px; line-height: 1; }

main { max-width: 1320px; margin: 0 auto; padding: 30px clamp(16px, 4vw, 40px) 60px; }
h2 { font-size: 18px; font-weight: 650; margin: 34px 0 12px; }
p.note { color: var(--muted); font-size: 15px; margin: 10px 0 0; max-width: 84ch; }

.cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 18px; }
.card {
  background: var(--surface); border: 1px solid var(--border); border-radius: 14px;
  padding: 20px 22px; box-shadow: var(--shadow);
}
.card .label { color: var(--muted); font-size: 15px; }
.card .figure { display: flex; align-items: center; flex-wrap: wrap; gap: 12px; margin-top: 4px; }
.card .percent { font-size: 42px; font-weight: 700; letter-spacing: -0.02em; font-variant-numeric: tabular-nums; line-height: 1.15; }
.card .counts { color: var(--muted); font-size: 15px; font-variant-numeric: tabular-nums; }

.bar { height: 9px; border-radius: 999px; background: var(--track); overflow: hidden; margin-top: 14px; }
.bar > span { display: block; height: 100%; border-radius: inherit; background: var(--accent); }
.bar.good > span { background: var(--good); }
.bar.warn > span { background: var(--warn); }
.bar.bad > span { background: var(--bad); }

.pill {
  display: inline-block; min-width: 4.2em; padding: 2px 10px; border-radius: 999px;
  font-size: 14px; font-weight: 600; text-align: center; font-variant-numeric: tabular-nums;
}
.pill.good { color: var(--good); background: var(--good-bg); }
.pill.warn { color: var(--warn); background: var(--warn-bg); }
.pill.bad { color: var(--bad); background: var(--bad-bg); }
.pill.none { color: var(--muted); background: var(--surface-alt); }

.toolbar { display: flex; align-items: center; justify-content: space-between; gap: 12px; flex-wrap: wrap; margin-top: 34px; }
.toolbar h2 { margin: 0; }
input.filter {
  width: min(340px, 100%); padding: 9px 13px; border-radius: 10px;
  border: 1px solid var(--border); background: var(--surface); color: var(--text); font: inherit; font-size: 15px;
}

.table-wrap {
  margin-top: 12px; background: var(--surface); border: 1px solid var(--border);
  border-radius: 14px; box-shadow: var(--shadow); overflow-x: auto;
}
table { border-collapse: collapse; width: 100%; }
th, td { padding: 12px 16px; text-align: left; border-bottom: 1px solid var(--border); vertical-align: middle; }
tr:last-child > td { border-bottom: 0; }
th {
  font-size: 13.5px; font-weight: 600; color: var(--muted); text-transform: uppercase; letter-spacing: 0.05em;
  background: var(--surface-alt); white-space: nowrap; user-select: none;
}
th[role="button"] { cursor: pointer; }
th[role="button"]::after { content: " \2195"; opacity: 0.45; }
th[aria-sort="ascending"]::after { content: " \2191"; opacity: 1; }
th[aria-sort="descending"]::after { content: " \2193"; opacity: 1; }
td { font-size: 15px; }
td.num { text-align: right; font-variant-numeric: tabular-nums; white-space: nowrap; }
th.num { text-align: right; }
/* Each metric is a fixed pill column followed by a fixed count column, so the rows line up. */
.metric {
  display: inline-grid; grid-template-columns: 6.6em 7.2em; column-gap: 0.8em; align-items: center;
}
.metric .pill { min-width: 0; width: 100%; }
.metric .counts { color: var(--muted); text-align: right; }
td.file a { font-weight: 550; }
td.bar-cell { width: 26%; min-width: 150px; }
td.bar-cell .bar { margin-top: 0; }
tbody tr:hover { background: var(--surface-alt); }

.source-meta { display: flex; gap: 12px; flex-wrap: wrap; margin: 20px 0 10px; }
.source-meta .chip {
  padding: 5px 12px; border-radius: 999px; background: var(--surface); border: 1px solid var(--border);
  font-size: 15px; color: var(--muted);
}
.legend { display: flex; gap: 16px; flex-wrap: wrap; font-size: 15px; color: var(--muted); margin: 8px 0 14px; }
.legend span::before {
  content: ""; display: inline-block; width: 12px; height: 12px; border-radius: 3px; margin-right: 7px; vertical-align: -1px;
  background: var(--c);
}
.uncovered-list { margin: 8px 0 0; padding-left: 20px; font-size: 15px; }
.uncovered-list li { margin: 4px 0; }

/* Source listing: line number, code, then the hit count at the right edge of the line. */
.source table { font-family: var(--mono-stack); font-size: 15px; width: 100%; }
.source col.c-ln { width: 3.8em; }
.source col.c-hits { width: 6.4em; }
.source td {
  padding: 0 0.8em; border: 0; vertical-align: top; line-height: 1.7;
  font-family: var(--mono-stack); font-size: 15px;
  font-variant-ligatures: none; font-feature-settings: "liga" 0, "calt" 0;
}
.source td.ln {
  text-align: right; color: var(--muted); user-select: none; white-space: nowrap;
  font-variant-numeric: tabular-nums; border-right: 1px solid var(--border);
  padding-right: 0.9em;
}
.source td.code { white-space: pre; tab-size: 4; padding-left: 1em; color: var(--text); }
.source td.hits {
  text-align: right; color: var(--muted); white-space: nowrap;
  font-variant-numeric: tabular-nums; padding-right: 1.2em;
}
.source tr.cov { background: var(--good-bg); }
.source tr.cov td.hits { color: var(--good); }
.source tr.part { background: var(--warn-bg); }
.source tr.part td.hits { color: var(--warn); }
.source tr.miss { background: var(--bad-bg); }
.source tr.miss td.hits { color: var(--bad); font-weight: 600; }
.source tr.excl td.code, .source tr.excl td.hits { color: var(--excluded); font-style: italic; }
.source tr.cov td.ln, .source tr.part td.ln, .source tr.miss td.ln { border-right-color: transparent; }
.source tr:hover td { background-color: color-mix(in srgb, var(--text) 6%, transparent); }

/* Syntax colors: Dracula and Alucard roles. */
.tk-kw { color: var(--tk-kw); }
.tk-ty { color: var(--tk-type); }
.tk-st { color: var(--tk-str); }
.tk-nu, .tk-cn { color: var(--tk-num); }
.tk-cm { color: var(--tk-com); }
.tk-fn { color: var(--tk-fn); }
.tk-pp { color: var(--tk-pre); }

footer { color: var(--muted); font-size: 14px; margin-top: 44px; }

@media (max-width: 640px) {
  header.top .meta { display: none; }
  th, td { padding: 10px 11px; }
  .card .percent { font-size: 34px; }
  .source table, .source td { font-size: 14px; }
  .source col.c-ln { width: 3.6em; }
  .source col.c-hits { width: 5.6em; }
}
"""

SCRIPT = r"""
(function () {
  var key = "__THEME_KEY__";
  var root = document.documentElement;

  function systemTheme() {
    return window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }

  function currentTheme() {
    return root.getAttribute("data-theme") || systemTheme();
  }

  function paintButton() {
    var button = document.getElementById("theme-toggle");
    if (!button) return;
    var dark = currentTheme() === "dark";
    button.setAttribute("aria-pressed", dark ? "true" : "false");
    button.querySelector(".label").textContent = dark ? "Switch to Alucard" : "Switch to Dracula";
    button.querySelector(".glyph").textContent = dark ? "\u2600" : "\u263E";
  }

  function setTheme(theme) {
    root.setAttribute("data-theme", theme);
    try { localStorage.setItem(key, theme); } catch (error) { /* storage may be blocked */ }
    paintButton();
  }

  document.addEventListener("DOMContentLoaded", function () {
    var button = document.getElementById("theme-toggle");
    if (button) {
      button.addEventListener("click", function () {
        setTheme(currentTheme() === "dark" ? "light" : "dark");
      });
    }
    paintButton();

    var filter = document.getElementById("file-filter");
    if (filter) {
      filter.addEventListener("input", function () {
        var needle = filter.value.trim().toLowerCase();
        var rows = document.querySelectorAll("tbody.files tr");
        for (var i = 0; i < rows.length; i++) {
          var name = rows[i].getAttribute("data-name");
          rows[i].hidden = needle !== "" && name.indexOf(needle) === -1;
        }
      });
    }

    var headers = document.querySelectorAll("th[role='button']");
    for (var h = 0; h < headers.length; h++) {
      headers[h].addEventListener("click", sortBy(headers[h]));
      headers[h].addEventListener("keydown", function (event) {
        if (event.key === "Enter" || event.key === " ") {
          event.preventDefault();
          this.click();
        }
      });
    }
  });

  function sortBy(header) {
    return function () {
      var table = header.closest("table");
      var body = table.tBodies[0];
      var index = Array.prototype.indexOf.call(header.parentNode.children, header);
      var descending = header.getAttribute("aria-sort") !== "descending";

      var all = table.querySelectorAll("th[role='button']");
      for (var i = 0; i < all.length; i++) all[i].removeAttribute("aria-sort");
      header.setAttribute("aria-sort", descending ? "descending" : "ascending");

      var rows = Array.prototype.slice.call(body.rows);
      rows.sort(function (a, b) {
        var left = a.cells[index].getAttribute("data-value");
        var right = b.cells[index].getAttribute("data-value");
        var numeric = !isNaN(parseFloat(left)) && !isNaN(parseFloat(right));
        var result = numeric ? parseFloat(left) - parseFloat(right) : left.localeCompare(right);
        return descending ? -result : result;
      });
      for (var r = 0; r < rows.length; r++) body.appendChild(rows[r]);
    };
  }
})();
"""

HEAD_SCRIPT = (
    "<script>try{var t=localStorage.getItem('" + THEME_KEY + "');"
    "if(t==='light'||t==='dark'){document.documentElement.setAttribute('data-theme',t);}}"
    "catch(e){}</script>"
)

TOGGLE = (
    '<button type="button" class="theme" id="theme-toggle" aria-pressed="false">'
    '<span class="glyph" aria-hidden="true"></span><span class="label">Switch theme</span></button>'
)


def rgba(hex_color, alpha):
    value = hex_color.lstrip("#")
    red, green, blue = int(value[0:2], 16), int(value[2:4], 16), int(value[4:6], 16)
    return f"rgba({red}, {green}, {blue}, {alpha})"


def theme_tokens(palette, dark):
    """Returns the CSS custom properties for one theme, derived from its palette."""
    shadow = (
        "0 1px 2px rgba(0, 0, 0, 0.35), 0 8px 24px rgba(0, 0, 0, 0.30)" if dark
        else "0 1px 2px rgba(31, 31, 31, 0.06), 0 6px 18px rgba(31, 31, 31, 0.06)"
    )
    surface = "#21222C" if dark else "#FFFFFF"
    return f"""
    --bg: {palette['bg']};
    --surface: {surface};
    --surface-alt: {rgba(palette['selection'], 0.45)};
    --text: {palette['fg']};
    --muted: {rgba(palette['fg'], 0.72)};
    --border: {palette['selection']};
    --accent: {palette['purple']};
    --good: {palette['green']};
    --good-bg: {rgba(palette['green'], 0.16)};
    --warn: {palette['yellow']};
    --warn-bg: {rgba(palette['yellow'], 0.20)};
    --bad: {palette['red']};
    --bad-bg: {rgba(palette['red'], 0.18)};
    --excluded: {palette['comment']};
    --track: {palette['selection']};
    --shadow: {shadow};
    --mono-stack: "JetBrains Mono", "Cascadia Code", ui-monospace, SFMono-Regular, Menlo, Consolas, "Liberation Mono", monospace;
    --tk-kw: {palette['pink']};
    --tk-type: {palette['cyan']};
    --tk-str: {palette['yellow']};
    --tk-num: {palette['purple']};
    --tk-com: {palette['comment']};
    --tk-fn: {palette['green']};
    --tk-pre: {palette['orange']};
    color-scheme: {'dark' if dark else 'light'};
"""


def style_sheet():
    return STYLE.replace("%%LIGHT%%", theme_tokens(ALUCARD, False)).replace(
        "%%DARK%%", theme_tokens(DRACULA, True)
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


def span(css_class, text):
    escaped = html.escape(text)
    return f'<span class="tk-{css_class}">{escaped}</span>' if css_class else escaped


def highlight_lines(lines):
    """Returns the HTML of each source line, with C++ tokens wrapped in classes.

    Block comments carry over from one line to the next. A preprocessor line is colored
    as a whole. Types are words after a class-like keyword or starting with a capital,
    and functions are words followed by a parenthesis. The highlighting is a light
    approximation, not a parser.
    """
    rendered = []
    in_block = False
    for line in lines:
        pieces = []
        pos = 0

        if in_block:
            end = line.find("*/")
            if end == -1:
                rendered.append(span("cm", line))
                continue
            pieces.append(span("cm", line[:end + 2]))
            pos = end + 2
            in_block = False

        if pos == 0 and line.lstrip().startswith("#"):
            rendered.append(span("pp", line))
            continue

        previous = ""
        while pos < len(line):
            match = TOKEN_RE.match(line, pos)
            kind, text = match.lastgroup, match.group(0)
            pos = match.end()
            css_class = None

            if kind == "comment":
                css_class = "cm"
                if text.startswith("/*") and (len(text) < 4 or not text.endswith("*/")):
                    in_block = True
            elif kind in ("string", "char"):
                css_class = "st"
            elif kind == "number":
                css_class = "nu"
            elif kind == "word":
                if text in CONSTANTS:
                    css_class = "cn"
                elif text in CPP_KEYWORDS:
                    css_class = "kw"
                elif previous in TYPE_INTRO or text[0].isupper():
                    css_class = "ty"
                elif line[pos:].lstrip().startswith("("):
                    css_class = "fn"

            if kind != "space":
                previous = text
            pieces.append(span(css_class, text))

        rendered.append("".join(pieces))
    return rendered


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


def page(title, body, crumb=None, meta=""):
    crumb_html = f'<div class="crumb">{crumb}</div>' if crumb else ""
    meta_html = f'<span class="meta">{html.escape(meta)}</span>' if meta else ""
    return f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="color-scheme" content="light dark">
<title>{html.escape(title)}</title>
{HEAD_SCRIPT}
<style>{style_sheet()}</style>
</head>
<body>
<header class="top">
  <div>
    <h1>HackySTL coverage</h1>
    {crumb_html}
  </div>
  <div style="display:flex;align-items:center;gap:16px">
    {meta_html}
    {TOGGLE}
  </div>
</header>
<main>
{body}
</main>
<script>{SCRIPT.replace("__THEME_KEY__", THEME_KEY)}</script>
</body>
</html>
"""


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
                f'<td class="num" data-value="{value:.4f}">'
                f'<span class="metric">{meter(covered, total)[0]}'
                f'<span class="counts">{covered:,}/{total:,}</span></span></td>'
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
        '<th class="num" role="button" tabindex="0">Lines</th>'
        '<th class="num" role="button" tabindex="0">Branches</th>'
        '<th class="num" role="button" tabindex="0">Functions</th>'
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
<footer>Click a column heading to sort. Percentages of 90% and above are good, 75% and above are fair,
and anything lower needs attention.</footer>
"""
    (out_dir / "index.html").write_text(
        page("HackySTL coverage", body, meta=generated), encoding="utf-8"
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
    highlighted = highlight_lines(source_lines)

    rows = []
    for number, code_html in enumerate(highlighted, start=1):
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
            f'<td class="code">{code_html}</td><td class="hits">{hits}</td></tr>'
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
  <colgroup><col class="c-ln"><col class="c-code"><col class="c-hits"></colgroup>
  <tbody>
    {''.join(rows)}
  </tbody>
</table>
</div>
"""
    crumb = f'<a href="../index.html">Summary</a> &rsaquo; <span class="mono">{html.escape(file_name)}</span>'
    (out_dir / file_page_name(file_name)).write_text(
        page(f"{file_name} - HackySTL coverage", body, crumb=crumb), encoding="utf-8"
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
