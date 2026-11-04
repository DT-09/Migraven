#!/usr/bin/env python3
"""Run Migraven end-to-end against the bundled synthetic migration exports."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def find_binary(explicit: str | None) -> Path:
    candidates = [Path(explicit)] if explicit else [
        ROOT / "build" / "migraven",
        ROOT / "build" / "migraven.exe",
        ROOT / "build" / "Release" / "migraven.exe",
    ]
    for candidate in candidates:
        resolved = candidate.resolve()
        if resolved.is_file():
            return resolved
    raise FileNotFoundError(
        "Migraven executable not found. Build it first with "
        "'cmake -S . -B build && cmake --build build', or pass --binary."
    )


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command, cwd=ROOT, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, check=True,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", help="Path to the compiled Migraven executable")
    parser.add_argument("--examples-dir", help="Directory containing migration_source.csv and migration_target.csv")
    parser.add_argument(
        "--output", default="migraven-demo-output",
        help="Directory for the JSON report, HTML report and evidence manifest",
    )
    args = parser.parse_args()

    try:
        binary = find_binary(args.binary)
        output = Path(args.output).resolve()
        output.mkdir(parents=True, exist_ok=True)

        examples = Path(args.examples_dir).resolve() if args.examples_dir else ROOT / "examples"
        left = examples / "migration_source.csv"
        right = examples / "migration_target.csv"
        report = output / "comparison.json"
        html_report = output / "review.html"
        manifest = output / "evidence.json"

        comparison = run([
            str(binary), "compare", "--key", "record_id",
            "--field", "account_status",
            str(left), str(right),
        ])
        parsed = json.loads(comparison.stdout)
        if parsed.get("status") not in {"PASS", "REVIEW"}:
            raise ValueError("The CLI returned an unrecognised report status")
        report.write_text(
            json.dumps(parsed, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
        )

        run([
            sys.executable, str(ROOT / "tools" / "render_report.py"),
            str(report), "--output", str(html_report),
        ])
        run([
            sys.executable, str(ROOT / "tools" / "evidence.py"), "create",
            "--left", str(left), "--right", str(right),
            "--report", str(report), "--key", "record_id",
            "--field", "account_status",
            "--format", "csv", "--tool-version", "0.1.0",
            "--output", str(manifest),
        ])
        verification = run([
            sys.executable, str(ROOT / "tools" / "evidence.py"),
            "verify", str(manifest),
        ])
        verified = json.loads(verification.stdout)
        if verified.get("status") != "VERIFIED":
            raise ValueError("Evidence manifest verification did not pass")

        print("MIGRAVEN DEMO COMPLETE")
        print(f"Comparison status: {parsed['status']}")
        print(f"Checks: {len(parsed.get('checks', []))}")
        print(f"Findings: {sum(len(c.get('findings', [])) for c in parsed.get('checks', []))}")
        print(f"HTML review: {html_report}")
        print(f"JSON result: {report}")
        print(f"Evidence manifest: {manifest}")
        print("Integrity check: VERIFIED (manifest authenticity is not established)")
        print("Dataset: synthetic demonstration records; not customer data")
        return 0
    except (OSError, subprocess.CalledProcessError, json.JSONDecodeError, ValueError) as exc:
        detail = getattr(exc, "stderr", None)
        print(f"migraven-demo: {exc}", file=sys.stderr)
        if detail:
            print(detail, file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
