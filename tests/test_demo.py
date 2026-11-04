#!/usr/bin/env python3
"""Integration test for the end-to-end Migraven demonstration package."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile


def main() -> int:
    script = Path(__file__).resolve().parents[1] / "tools" / "demo.py"
    binary = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory() as temporary:
        output = Path(temporary) / "demo"
        result = subprocess.run(
            [sys.executable, str(script), "--binary", str(binary), "--output", str(output)],
            text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        if result.returncode != 0:
            raise SystemExit(result.stderr or result.stdout)
        report = json.loads((output / "comparison.json").read_text(encoding="utf-8"))
        manifest = json.loads((output / "evidence.json").read_text(encoding="utf-8"))
        html = (output / "review.html").read_text(encoding="utf-8")
        if report.get("status") != "REVIEW":
            raise SystemExit("Synthetic demo should produce REVIEW status")
        if not report.get("checks") or not manifest.get("files"):
            raise SystemExit("Demo outputs are incomplete")
        if "Migration verification report" not in html:
            raise SystemExit("HTML report was not rendered")
        if "VERIFIED" not in result.stdout:
            raise SystemExit("Evidence verification was not reported")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
