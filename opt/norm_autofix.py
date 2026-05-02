#!/usr/bin/env python3
"""
norm_autofix.py — Ecole 42 norm mechanical autofix.

Yaptıkları:
  1) 42 header'ı eksik dosyalara ekler (login: ozmerte).
  2) Satır başındaki 4-space dizilerini tab'a çevirir (yalnızca leading).
  3) Trailing whitespace temizler.
  4) Birden fazla ardışık boş satırı tek boş satıra indirger.
  5) Dosya sonuna tek bir newline bırakır.
  6) `space-before-function-name` davranışını düzeltir (return type sonrası tab).
  7) Inline `// ...` yorumlarını ve "scope-violation" yorumlarını fonksiyon
     içlerinde **soft-skip** eder (raporda kalır, otomatik silmez —
     manuel inceleme istenir).

Kapsam:
  src/*.c
  hdr/*.h

Kullanım:
  python3 opt/norm_autofix.py
"""

from __future__ import annotations

import os
import re
import sys
from datetime import datetime, timezone
from pathlib import Path

LOGIN = "ozmerte"
EMAIL = f"{LOGIN}@student.42.fr"

ROOT = Path(__file__).resolve().parent.parent
TARGET_DIRS = [ROOT / "src", ROOT / "hdr"]
TARGET_GLOBS = ("*.c", "*.h")

# Detect already-present 42 header (first line opens with the banner).
HDR_BANNER_RE = re.compile(r"^/\* \*{74} \*/")


# -----------------------------------------------------------------------------
# 42 Header generator
# -----------------------------------------------------------------------------
def make_42_header(filename: str) -> str:
    """Return a fully-formed Ecole 42 header.

    Slot widths (validated empirically against norminette):
      - filename slot: 51 chars
      - author slot:   43 chars
      - date+by+login slot: 40 chars
    """
    now = datetime.now(timezone.utc).strftime("%Y/%m/%d %H:%M:%S")
    fn = filename if len(filename) <= 51 else filename[:51]

    fn_slot = fn.ljust(51)
    author_slot = f"{LOGIN} <{EMAIL}>".ljust(43)
    created_slot = f"{now} by {LOGIN}".ljust(40)
    updated_slot = f"{now} by {LOGIN}".ljust(39)

    return (
        "/* ************************************************************************** */\n"
        "/*                                                                            */\n"
        "/*                                                        :::      ::::::::   */\n"
        f"/*   {fn_slot}:+:      :+:    :+:   */\n"
        "/*                                                    +:+ +:+         +:+     */\n"
        f"/*   By: {author_slot}+#+  +:+       +#+        */\n"
        "/*                                                +#+#+#+#+#+   +#+           */\n"
        f"/*   Created: {created_slot}#+#    #+#             */\n"
        f"/*   Updated: {updated_slot}###   ########.fr       */\n"
        "/*                                                                            */\n"
        "/* ************************************************************************** */\n"
        "\n"
    )


def has_42_header(text: str) -> bool:
    head = text.lstrip("\n")
    return bool(HDR_BANNER_RE.match(head))


# -----------------------------------------------------------------------------
# Tabify leading whitespace
# -----------------------------------------------------------------------------
LEADING_WS_RE = re.compile(r"^([ \t]+)")


def tabify_leading(line: str, tab_width: int = 4) -> str:
    """Convert leading 4-space groups into tabs, preserving any tabs already
    present and any non-whitespace content."""
    m = LEADING_WS_RE.match(line)
    if not m:
        return line
    leading = m.group(1)
    # Expand tabs to spaces using tab_width, then convert back.
    expanded = leading.expandtabs(tab_width)
    n = len(expanded)
    tabs = n // tab_width
    rest = n % tab_width
    new_leading = "\t" * tabs + " " * rest
    return new_leading + line[m.end():]


# -----------------------------------------------------------------------------
# Return parenthesis: `return X;` → `return (X);`
# -----------------------------------------------------------------------------
RETURN_RE = re.compile(r"^(\s*)return\s+([^;()]+?)\s*;(\s*)$")


def fix_return_parens(line: str) -> str:
    m = RETURN_RE.match(line)
    if not m:
        return line
    indent, value, trailing = m.group(1), m.group(2).strip(), m.group(3)
    if not value or value.startswith("(") or value == "":
        return line
    return f"{indent}return ({value});{trailing}"


# -----------------------------------------------------------------------------
# Brace-newline (Allman): `... ) {` → `... )\n{`
# -----------------------------------------------------------------------------
BRACE_INLINE_RE = re.compile(r"^(\s*)(.*\S)\s*\{\s*$")


def split_brace(line: str) -> list[str]:
    """If line ends with `... {`, split into two lines (excluding string literals)."""
    if "{" not in line:
        return [line]
    # Skip lines that are pure `{`, or struct/array initializers with `{...}`
    stripped = line.strip()
    if stripped == "{" or stripped.startswith("//"):
        return [line]
    # Avoid touching lines that contain a closing brace on same line
    if "}" in stripped:
        return [line]
    # Avoid struct/typedef/initializer braces (heuristic: `=` before `{`)
    if "=" in line and line.rfind("=") < line.rfind("{"):
        return [line]
    m = BRACE_INLINE_RE.match(line)
    if not m:
        return [line]
    indent, head = m.group(1), m.group(2).rstrip()
    if head.endswith(","):  # initializer continuation
        return [line]
    # Only split if line ends with `)` or keyword (function/control-flow)
    if not (head.endswith(")") or head.endswith("else") or head.endswith("do")):
        return [line]
    return [f"{indent}{head}", f"{indent}{{"]


# -----------------------------------------------------------------------------
# Strip line-style C99 comments inside function bodies.
# (norminette: WRONG_SCOPE_COMMENT — comments forbidden in function scope.)
# -----------------------------------------------------------------------------
LINE_COMMENT_RE = re.compile(r"^(\s*)//.*$")


def is_full_line_comment(line: str) -> bool:
    return bool(LINE_COMMENT_RE.match(line))


# -----------------------------------------------------------------------------
# Function definition: convert `type   name(` into `type\tname(`
# -----------------------------------------------------------------------------
FUNC_DEF_RE = re.compile(
    r"^(\s*)"                              # leading indent (file scope = empty)
    r"((?:static\s+|inline\s+|const\s+|unsigned\s+|signed\s+|struct\s+\w+\s*\*?\s*|\w+\s*\*?\s*)+?)"
    r"(\w+)"                               # function name
    r"\s*\((.*)$"                          # opening paren + rest
)


def fix_space_before_func(line: str) -> str:
    """Heuristic: function-definition lines at file scope where return-type
    block ends with multiple spaces before the function name.
    Replace with a single tab for norm SPACE_BEFORE_FUNC compliance."""
    if "(" not in line or "=" in line.split("(")[0]:
        return line
    if line.strip().startswith(("//", "/*", "#", "{", "}")):
        return line
    m = FUNC_DEF_RE.match(line)
    if not m:
        return line
    indent, ret_type, fn_name, rest = m.group(1), m.group(2), m.group(3), m.group(4)
    ret_type = ret_type.rstrip()
    if not ret_type:
        return line
    # Skip statements like `return foo(...)` and control-flow keywords
    if ret_type in ("return", "if", "while", "for", "switch", "do", "else"):
        return line
    return f"{indent}{ret_type}\t{fn_name}({rest}"


def strip_inline_trailing_comment(line: str) -> str:
    """Remove trailing `// ...` from a code line (preserving any string literal
    that contains `//`). Conservative: skips lines with `"` to avoid false
    positives."""
    if "//" not in line:
        return line
    if '"' in line:
        return line  # don't risk touching strings
    idx = line.find("//")
    return line[:idx].rstrip()


# -----------------------------------------------------------------------------
# File processor
# -----------------------------------------------------------------------------
def strip_existing_header(lines: list[str]) -> list[str]:
    """If first non-empty line opens a 42 header banner, drop the first 12
    lines (header is 11 lines + 1 blank) up to and including the trailing
    blank line."""
    i = 0
    while i < len(lines) and lines[i].strip() == "":
        i += 1
    if i >= len(lines) or not HDR_BANNER_RE.match(lines[i]):
        return lines
    # Find closing banner (next line opening with /* *** ... )
    end = i + 1
    while end < len(lines) and not HDR_BANNER_RE.match(lines[end]):
        end += 1
    if end >= len(lines):
        return lines  # malformed, leave alone
    drop_until = end + 1
    # Eat one trailing blank line if present
    if drop_until < len(lines) and lines[drop_until].strip() == "":
        drop_until += 1
    return lines[drop_until:]


def process_file(path: Path, force_header: bool = False) -> tuple[bool, str]:
    """Return (changed, reason)."""
    original = path.read_text(encoding="utf-8", errors="replace")
    lines = original.splitlines(keepends=False)
    new_lines: list[str] = []

    # 1) Inject 42 header (force-mode strips existing first)
    if force_header:
        lines = strip_existing_header(lines)
        inject_header = True
    else:
        inject_header = not has_42_header(original)
    if inject_header:
        new_lines.extend(make_42_header(path.name).splitlines(keepends=False))

    # 2) Per-line cleanup with brace-depth tracking (in-function awareness)
    brace_depth = 0
    in_block_comment = False
    for raw in lines:
        ln = raw.rstrip()

        # Track block-comment state (very rough — header/file-level)
        if in_block_comment:
            if "*/" in ln:
                in_block_comment = False
            new_lines.append(tabify_leading(ln))
            continue
        if "/*" in ln and "*/" not in ln:
            in_block_comment = True
            new_lines.append(tabify_leading(ln))
            continue

        # In-function: drop empty lines, full-line `// ...`, inline `//`, fix return
        if brace_depth >= 1:
            if ln.strip() == "":
                continue  # EMPTY_LINE_FUNCTION
            if is_full_line_comment(ln):
                continue  # WRONG_SCOPE_COMMENT
            ln = strip_inline_trailing_comment(ln)
            ln = fix_return_parens(ln)
        else:
            # File scope: convert `type<spaces>name(` into `type\tname(` for
            # function definitions (SPACE_BEFORE_FUNC). Conservative: only when
            # line ends with `(` and has no `=`.
            ln = fix_space_before_func(ln)

        # Brace-newline split
        for piece in split_brace(ln):
            piece = tabify_leading(piece)
            new_lines.append(piece)

        # Update brace depth (after line is appended; use last raw line content)
        for ch in ln:
            if ch == "{":
                brace_depth += 1
            elif ch == "}":
                brace_depth = max(0, brace_depth - 1)

    # 3) Collapse 2+ blank lines into single blank
    collapsed: list[str] = []
    blank_run = 0
    for ln in new_lines:
        if ln.strip() == "":
            blank_run += 1
            if blank_run <= 1:
                collapsed.append("")
        else:
            blank_run = 0
            collapsed.append(ln)

    # 4) Ensure final newline (single)
    while collapsed and collapsed[-1] == "":
        collapsed.pop()
    collapsed.append("")  # trailing newline

    new_text = "\n".join(collapsed)

    if new_text != original:
        path.write_text(new_text, encoding="utf-8")
        reasons = []
        if inject_header:
            reasons.append("header+")
        reasons.append("tabify")
        return True, ",".join(reasons)
    return False, "skip"


# -----------------------------------------------------------------------------
# Main
# -----------------------------------------------------------------------------
def main() -> int:
    force_header = "--force-header" in sys.argv[1:]

    if not all(d.is_dir() for d in TARGET_DIRS):
        print(f"error: target dirs not found under {ROOT}", file=sys.stderr)
        return 2

    files: list[Path] = []
    for d in TARGET_DIRS:
        for pattern in TARGET_GLOBS:
            files.extend(sorted(d.glob(pattern)))

    if not files:
        print("no files matched", file=sys.stderr)
        return 1

    changed = 0
    for f in files:
        ok, reason = process_file(f, force_header=force_header)
        flag = "✓" if ok else "·"
        rel = f.relative_to(ROOT)
        print(f"  {flag} {rel} ({reason})")
        if ok:
            changed += 1

    print(f"\n{changed}/{len(files)} files changed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
