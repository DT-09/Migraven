#!/usr/bin/env python3
"""Create and verify SHA-256 evidence manifests for Migraven comparison runs."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
from datetime import datetime, timezone

SCHEMA = "migraven-evidence/v1"


def sha256_file(path: Path) -> tuple[str, int]:
    digest = hashlib.sha256()
    size = 0
    with path.open("rb") as stream:
        while True:
            chunk = stream.read(1024 * 1024)
            if not chunk:
                break
            digest.update(chunk)
            size += len(chunk)
    return digest.hexdigest(), size


def stored_path(path: Path, manifest_dir: Path) -> str:
    resolved = path.resolve()
    try:
        return os.path.relpath(resolved, manifest_dir.resolve()).replace(os.sep, "/")
    except ValueError:
        # Windows cannot express a relative path across different drive letters.
        # Preserve a usable local reference and make its non-portability explicit.
        return str(resolved)


def create(args: argparse.Namespace) -> int:
    manifest_path = Path(args.output).resolve()
    manifest_dir = manifest_path.parent
    left = Path(args.left).resolve()
    right = Path(args.right).resolve()
    report = Path(args.report).resolve()

    if manifest_path in {left, right, report}:
        raise ValueError("Manifest output must not overwrite an input or comparison report")
    if len(set(args.field)) != len(args.field):
        raise ValueError("Duplicate comparison fields are not allowed")
    if not args.key or not args.field:
        raise ValueError("A comparison key and at least one field are required")

    try:
        report_data = json.loads(report.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"Cannot read comparison report: {exc}") from exc
    if not isinstance(report_data, dict) or report_data.get("status") not in {"PASS", "REVIEW"}:
        raise ValueError("Report is not a valid Migraven comparison result")
    if report_data.get("key") != args.key or report_data.get("format") != args.format:
        raise ValueError("Manifest key/format do not match the comparison report")
    checks = report_data.get("checks")
    if not isinstance(checks, list) or any(not isinstance(item, dict) for item in checks):
        raise ValueError("Report contains an invalid checks array")
    if [item.get("field") for item in checks] != args.field:
        raise ValueError("Manifest fields do not match the comparison report")

    for label, path in (("left input", left), ("right input", right), ("report", report)):
        if not path.is_file():
            raise ValueError(f"{label} is not a readable regular file: {path}")

    files = {}
    for label, path in (("left", left), ("right", right), ("report", report)):
        digest, size = sha256_file(path)
        files[label] = {
            "path": stored_path(path, manifest_dir),
            "sha256": digest,
            "size_bytes": size,
        }

    fields = list(args.field)

    manifest = {
        "schema": SCHEMA,
        "created_utc": datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z"),
        "tool": {"name": "Migraven", "version": args.tool_version},
        "comparison": {
            "format": args.format,
            "key": args.key,
            "fields": fields,
            "semantics": "exact-string-equality",
        },
        "files": files,
        "integrity_note": (
            "SHA-256 detects changes relative to this manifest. This manifest is not "
            "digitally signed; authenticity requires a trusted external copy or signature."
        ),
    }

    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = manifest_path.with_suffix(manifest_path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    temporary.replace(manifest_path)
    print(json.dumps({"status": "CREATED", "manifest": str(manifest_path), "files": files}, indent=2))
    return 0


def verify(args: argparse.Namespace) -> int:
    manifest_path = Path(args.manifest).resolve()
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"Cannot read manifest: {exc}") from exc

    if not isinstance(manifest, dict) or manifest.get("schema") != SCHEMA:
        raise ValueError(f"Unsupported or invalid manifest schema; expected {SCHEMA}")

    files = manifest.get("files")
    if not isinstance(files, dict) or set(files) != {"left", "right", "report"}:
        raise ValueError("Manifest must contain left, right and report file records")

    results = []
    failed = False
    for label in ("left", "right", "report"):
        record = files[label]
        if not isinstance(record, dict):
            raise ValueError(f"Invalid {label} file record")
        relative = record.get("path")
        expected_hash = record.get("sha256")
        expected_size = record.get("size_bytes")
        if not isinstance(relative, str) or not relative:
            raise ValueError(f"Invalid path for {label}")
        if not isinstance(expected_hash, str) or len(expected_hash) != 64:
            raise ValueError(f"Invalid SHA-256 value for {label}")
        if not isinstance(expected_size, int) or expected_size < 0:
            raise ValueError(f"Invalid size for {label}")

        recorded_path = Path(relative)
        path = recorded_path.resolve() if recorded_path.is_absolute() else (manifest_path.parent / recorded_path).resolve()
        try:
            actual_hash, actual_size = sha256_file(path)
            status = "PASS" if actual_hash == expected_hash and actual_size == expected_size else "FAIL"
            actual = {"sha256": actual_hash, "size_bytes": actual_size}
        except OSError as exc:
            status = "MISSING"
            actual = {"error": str(exc)}
        if status != "PASS":
            failed = True
        results.append({"file": label, "status": status, "expected": {
            "sha256": expected_hash, "size_bytes": expected_size
        }, "actual": actual})

    output = {
        "status": "VERIFIED" if not failed else "TAMPERED_OR_MISSING",
        "manifest": str(manifest_path),
        "checks": results,
        "authenticity": "NOT_ESTABLISHED: manifest is unsigned",
    }
    print(json.dumps(output, indent=2))
    return 1 if failed else 0


def parser() -> argparse.ArgumentParser:
    root = argparse.ArgumentParser(description="Create or verify Migraven SHA-256 evidence manifests")
    commands = root.add_subparsers(dest="command", required=True)

    make = commands.add_parser("create", help="Create a manifest for one comparison run")
    make.add_argument("--left", required=True)
    make.add_argument("--right", required=True)
    make.add_argument("--report", required=True)
    make.add_argument("--key", required=True)
    make.add_argument("--field", action="append", required=True)
    make.add_argument("--format", choices=("csv", "jsonl"), default="csv")
    make.add_argument("--tool-version", default="0.1.0")
    make.add_argument("--output", required=True)
    make.set_defaults(func=create)

    check = commands.add_parser("verify", help="Verify all files recorded in a manifest")
    check.add_argument("manifest")
    check.set_defaults(func=verify)
    return root


def main() -> int:
    args = parser().parse_args()
    try:
        return args.func(args)
    except (OSError, ValueError) as exc:
        print(f"migraven-evidence: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
