#!/usr/bin/env python3
"""Test release archive contents and its published SHA-256 checksum."""

import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
binary = Path(sys.argv[1]).resolve()

with tempfile.TemporaryDirectory(prefix="migraven package ") as temporary:
    output = Path(temporary) / "dist"
    subprocess.run(
        [
            sys.executable, str(ROOT / "tools" / "package_release.py"),
            "--binary", str(binary),
            "--version", "test",
            "--platform", "test-platform",
            "--output-dir", str(output),
        ],
        check=True,
        text=True,
        capture_output=True,
    )
    archive = output / "Migraven-test-test-platform.zip"
    checksum = output / f"{archive.name}.sha256"
    assert archive.is_file()
    assert checksum.is_file()
    checksum_line = checksum.read_text(encoding="ascii").strip()
    expected, filename = checksum_line.split(maxsplit=1)
    assert filename == archive.name
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    assert actual == expected

    executable = "migraven.exe" if binary.suffix.lower() == ".exe" else "migraven"
    with zipfile.ZipFile(archive) as bundle:
        names = set(bundle.namelist())
        required = {
            f"Migraven-test-test-platform/bin/{executable}",
            "Migraven-test-test-platform/README.md",
            "Migraven-test-test-platform/docs/customer-quickstart.md",
            "Migraven-test-test-platform/tools/evidence.py",
            "Migraven-test-test-platform/demo-output/comparison.json",
            "Migraven-test-test-platform/demo-output/review.html",
            "Migraven-test-test-platform/demo-output/evidence.json",
        }
        assert required.issubset(names), required - names
        assert bundle.testzip() is None

print("release package test passed")
