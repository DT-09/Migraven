import json
import pathlib
import subprocess
import sys
import tempfile

VERITY = sys.argv[1]

def run(*args):
    return subprocess.run([VERITY, *args], text=True, capture_output=True, check=False)

with tempfile.TemporaryDirectory(prefix="verity cli ") as directory:
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
    assert len(report["checks"]) == 2

    status = report["checks"][0]
    assert status["field"] == "status"
    assert status["summary"] == {
        "matched": 2, "left_only": 1, "right_only": 1, "exceptions": 3
    }
    assert status["findings"][0]["key"] == "B"
    assert status["findings"][0]["status"] == "FAIL"

    region = report["checks"][1]
    assert region["field"] == "region"
    assert region["summary"]["exceptions"] == 2

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

print("all CLI tests passed")
