#!/usr/bin/env python3
"""Verify that the release packager creates a runnable, checksummed bundle."""

import hashlib
import pathlib
import subprocess
import sys
import tempfile
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
BINARY = pathlib.Path(sys.argv[1]).resolve()

with tempfile.TemporaryDirectory(prefix="verity package ") as temporary:
    output = pathlib.Path(temporary)
    result = subprocess.run(
        [
            sys.executable,
            str(ROOT / "tools" / "package_release.py"),
            "--binary", str(BINARY),
            "--version", "test",
            "--platform", "ci",
            "--output-dir", str(output),
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stdout + result.stderr

    archive = output / "Verity-test-ci.zip"
    checksum_file = output / "Verity-test-ci.zip.sha256"
    assert archive.is_file()
    assert checksum_file.is_file()
    checksum_line = checksum_file.read_text(encoding="ascii")
    assert checksum_line.endswith("\n")
    expected_hash, expected_name = checksum_line.strip().split(maxsplit=1)
    assert expected_name == archive.name
    assert expected_hash == hashlib.sha256(archive.read_bytes()).hexdigest()

    with zipfile.ZipFile(archive) as bundle:
        names = set(bundle.namelist())
        required = {
            "Verity-test-ci/bin/verity" if BINARY.suffix.lower() != ".exe"
            else "Verity-test-ci/bin/verity.exe",
            "Verity-test-ci/README.md",
            "Verity-test-ci/docs/data-handling.md",
            "Verity-test-ci/examples/migration_source.csv",
            "Verity-test-ci/tools/demo.py",
            "Verity-test-ci/demo-output/comparison.json",
            "Verity-test-ci/demo-output/review.html",
            "Verity-test-ci/demo-output/evidence.json",
        }
        assert required.issubset(names), sorted(required - names)

print("all release packaging tests passed")
