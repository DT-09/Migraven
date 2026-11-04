#!/usr/bin/env python3
"""Edge-case regression tests for CSV ingestion."""

import json
import pathlib
import subprocess
import sys
import tempfile

BINARY = pathlib.Path(sys.argv[1]).resolve()


def run(left: pathlib.Path, right: pathlib.Path):
    return subprocess.run(
        [str(BINARY), "compare", "--key", "id", "--field", "value",
         str(left), str(right)],
        text=True, capture_output=True, check=False,
    )


with tempfile.TemporaryDirectory(prefix="migraven csv edges ") as directory:
    root = pathlib.Path(directory)
    valid = root / "valid.csv"
    valid.write_text("id,value\nA,one\n", encoding="utf-8")

    bom = root / "bom.csv"
    bom.write_bytes(b"\xef\xbb\xbf" + b"id,value\nA,one\n")
    accepted_bom = run(bom, valid)
    assert accepted_bom.returncode == 0, accepted_bom.stderr
    assert json.loads(accepted_bom.stdout)["status"] == "PASS"

    malformed_cases = {
        "unterminated_quote": b'id,value\nA,"unterminated\n',
        "nul_byte": b"id,value\nA,one\x00two\n",
        "duplicate_header": b"id,id\nA,A\n",
        "empty_header": b"id,\nA,B\n",
        "wrong_width": b"id,value\nA,one,extra\n",
        "characters_after_quote": b'id,value\nA,"one"x\n',
    }
    for name, payload in malformed_cases.items():
        candidate = root / f"{name}.csv"
        candidate.write_bytes(payload)
        rejected = run(candidate, valid)
        assert rejected.returncode == 2, (
            f"{name} was not rejected: stdout={rejected.stdout!r} "
            f"stderr={rejected.stderr!r}"
        )

print("all CSV edge-case tests passed")
