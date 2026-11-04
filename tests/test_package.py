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

with tempfile.TemporaryDirectory(prefix="verity package ") as temporary:
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
    archive = output / "Verity-test-test-platform.zip"
    checksum = output / f"{archive.name}.sha256"
    assert archive.is_file()
    assert checksum.is_file()
    checksum_line = checksum.read_text(encoding="ascii").strip()
    expected, filename = checksum_line.split(maxsplit=1)
    assert filename == archive.name
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    assert actual == expected

    with zipfile.ZipFile(archive) as bundle:
        names = set(bundle.namelist())
        required = {
            "Verity-test-test-platform/bin/verity",
            "Verity-test-test-platform/README.md",
            "Verity-test-test-platform/docs/customer-quickstart.md",
            "Verity-test-test-platform/tools/evidence.py",
            "Verity-test-test-platform/demo-output/comparison.json",
            "Verity-test-test-platform/demo-output/review.html",
            "Verity-test-test-platform/demo-output/evidence.json",
        }
        assert required.issubset(names), required - names
        assert bundle.testzip() is None

print("release package test passed")
