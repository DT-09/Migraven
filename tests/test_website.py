#!/usr/bin/env python3
"""Smoke-test Migraven's static product website without third-party packages."""
from __future__ import annotations

from html.parser import HTMLParser
from pathlib import Path
import re
import sys
from urllib.parse import urlsplit

ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / "website"
PAGES = ("index.html", "product.html", "method.html", "evaluation.html")


class PageParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.links: list[tuple[str, str]] = []
        self.ids: set[str] = set()
        self.nav_current = 0
        self.menu_controls: list[str] = []
        self.menu_targets: list[str] = []
        self.in_nav = False

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        values = dict(attrs)
        if values.get("id"):
            self.ids.add(values["id"] or "")
        if tag in {"a", "link", "script"}:
            key = "href" if tag in {"a", "link"} else "src"
            value = values.get(key)
            if value:
                self.links.append((tag, value))
        if tag == "nav":
            self.in_nav = True
        if tag == "a" and self.in_nav and values.get("aria-current") == "page":
            self.nav_current += 1
        if tag == "button" and "menu-toggle" in (values.get("class") or ""):
            self.menu_controls.append(values.get("aria-controls") or "")
        if tag == "nav" and values.get("id"):
            self.menu_targets.append(values["id"])

    def handle_endtag(self, tag: str) -> None:
        if tag == "nav":
            self.in_nav = False


def fail(message: str) -> None:
    print(f"website test: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> int:
    if not SITE.is_dir():
        fail("website directory is missing")
    for name in PAGES:
        page = SITE / name
        if not page.is_file():
            fail(f"missing page: {name}")
        parser = PageParser()
        parser.feed(page.read_text(encoding="utf-8"))
        parser.close()
        if parser.nav_current != 1:
            fail(f"{name}: expected exactly one current navigation item")
        if not parser.menu_controls or not set(parser.menu_controls).issubset(set(parser.menu_targets)):
            fail(f"{name}: mobile menu control does not target an existing navigation region")
        for tag, href in parser.links:
            parsed = urlsplit(href)
            if parsed.scheme or parsed.netloc or href.startswith("#"):
                continue
            target = (page.parent / parsed.path).resolve()
            if not target.is_file():
                fail(f"{name}: broken local {tag} reference: {href}")
            if parsed.fragment and target.suffix == ".html":
                target_parser = PageParser()
                target_parser.feed(target.read_text(encoding="utf-8"))
                if parsed.fragment not in target_parser.ids:
                    fail(f"{name}: missing fragment {href}")

    css = (SITE / "assets/site.css").read_text(encoding="utf-8")
    if "@import" in css or re.search(r"url\(\s*['\"]?https?://", css, re.I):
        fail("stylesheet must not fetch third-party assets")
    js = (SITE / "assets/site.js").read_text(encoding="utf-8")
    if "http://" in js or "https://" in js:
        fail("site JavaScript must not call external services")
    print("Migraven website smoke tests passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
