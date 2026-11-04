from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import urlparse
import sys

ROOT = Path(__file__).resolve().parents[1] / "website"
PAGES = sorted(ROOT.glob("*.html"))

class PageParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []
        self.ids = set()
        self.title = False
        self.main = 0
        self.nav = 0
        self.h1 = 0
    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if attrs.get("id"):
            self.ids.add(attrs["id"])
        if tag in ("a", "link", "script") and attrs.get("href") or tag == "script" and attrs.get("src"):
            value = attrs.get("href") or attrs.get("src")
            self.links.append(value)
        if tag == "main": self.main += 1
        if tag == "nav": self.nav += 1
        if tag == "h1": self.h1 += 1

errors = []
expected = {"index.html", "product.html", "methodology.html", "evaluation.html", "faq.html"}
actual = {p.name for p in PAGES}
if actual != expected:
    errors.append(f"Expected pages {sorted(expected)}, found {sorted(actual)}")

for page in PAGES:
    parser = PageParser()
    parser.feed(page.read_text(encoding="utf-8"))
    if parser.main != 1:
        errors.append(f"{page.name}: expected exactly one main landmark")
    if parser.nav != 1:
        errors.append(f"{page.name}: expected exactly one primary nav")
    if parser.h1 != 1:
        errors.append(f"{page.name}: expected exactly one h1")
    for raw in parser.links:
        parsed = urlparse(raw)
        if parsed.scheme or raw.startswith("//") or raw.startswith("mailto:") or raw.startswith("tel:"):
            continue
        target = (page.parent / parsed.path).resolve() if parsed.path else page.resolve()
        if not target.exists():
            errors.append(f"{page.name}: missing local target {raw}")
            continue
        if parsed.fragment and target.suffix == ".html":
            target_parser = PageParser()
            target_parser.feed(target.read_text(encoding="utf-8"))
            if parsed.fragment not in target_parser.ids:
                errors.append(f"{page.name}: missing fragment {raw}")

css = ROOT / "assets" / "site.css"
js = ROOT / "assets" / "site.js"
if not css.is_file(): errors.append("Missing assets/site.css")
if not js.is_file(): errors.append("Missing assets/site.js")

if errors:
    print("\n".join(errors), file=sys.stderr)
    raise SystemExit(1)
print(f"Validated {len(PAGES)} pages, local links, landmarks and static assets.")
