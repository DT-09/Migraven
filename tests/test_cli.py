import json
import pathlib
import subprocess
import sys
import tempfile

MIGRAVEN = sys.argv[1]

def run(*args):
    return subprocess.run([MIGRAVEN, *args], text=True, capture_output=True, check=False)

with tempfile.TemporaryDirectory(prefix="migraven cli ") as directory:
    root = pathlib.Path(directory)
    left = root / "source data.csv"
    right = root / "target data.csv"
    left.write_text(
        "id,status,region\nA,ACTIVE,EU\nB,PENDING,US\nC,ACTIVE,APAC\n",
        encoding="utf-8",
    )
    right.write_text(
        "id,status,region\nA,ACTIVE,EU\nB,ACTIVE,US\nD,ACTIVE,APAC\n",
        encoding="utf-8",
    )

    result = run(
        "compare", "--key", "id",
        "--field", "status", "--field", "region",
        str(left), str(right),
    )
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)
    assert report["status"] == "REVIEW"
    assert report["key"] == "id"
    assert report["format"] == "csv"
    assert report["inputs"] == {
        "source": {"file": "source data.csv", "rows": 3},
        "target": {"file": "target data.csv", "rows": 3},
    }
    assert len(report["checks"]) == 2
    assert report["checks"][0]["summary"] == {
        "matched": 2, "left_only": 1, "right_only": 1, "exceptions": 3
    }
    assert report["checks"][0]["findings"][0]["key"] == "B"
    assert report["checks"][0]["findings"][0]["status"] == "FAIL"
    assert report["checks"][1]["summary"]["exceptions"] == 2\n\n    strict = run(\n        "compare", "--strict", "--key", "id",\n        "--field", "status", str(left), str(right),\n    )\n    assert strict.returncode == 1\n    assert json.loads(strict.stdout)["status"] == "REVIEW"

    duplicate = run(
        "compare", "--key", "id", "--field", "status",
        "--field", "status", str(left), str(right),
    )
    assert duplicate.returncode == 2
    assert "duplicate field" in duplicate.stderr

    missing = run(
        "compare", "--key", "id", "--field", "missing",
        str(left), str(right),
    )
    assert missing.returncode == 2
    assert "column not found" in missing.stderr

    jsonl_left = root / "source.jsonl"
    jsonl_right = root / "target.jsonl"
    jsonl_left.write_text(
        '{"id":"A","status":"ACTIVE","region":"EU"}\n'
        '{"region":"US","status":"PENDING","id":"B"}\n',
        encoding="utf-8",
    )
    jsonl_right.write_text(
        '{"id":"A","status":"ACTIVE","region":"EU"}\n'
        '{"status":"ACTIVE","id":"B","region":"US"}\n',
        encoding="utf-8",
    )
    jsonl_result = run(
        "compare", "--format", "jsonl", "--key", "id",
        "--field", "status", "--field", "region",
        str(jsonl_left), str(jsonl_right),
    )
    assert jsonl_result.returncode == 0, jsonl_result.stderr
    jsonl_report = json.loads(jsonl_result.stdout)
    assert jsonl_report["format"] == "jsonl"
    assert jsonl_report["status"] == "REVIEW"
    assert jsonl_report["checks"][0]["summary"]["exceptions"] == 1

    inspected = run("inspect", "--format", "jsonl", str(jsonl_left))
    assert inspected.returncode == 0, inspected.stderr
    assert json.loads(inspected.stdout)["row_count"] == 2

print("all CLI tests passed")
