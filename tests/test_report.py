import json
import pathlib
import subprocess
import sys
import tempfile

SCRIPT = pathlib.Path(__file__).resolve().parents[1] / "tools" / "render_report.py"
REPORT = {
    "status": "REVIEW",
    "key": "record_id",
    "format": "csv",
    "inputs": {
        "source": {"file": "legacy.csv", "rows": 18},
        "target": {"file": "new.csv", "rows": 18},
    },
    "checks": [{
        "field": "account_status",
        "summary": {"matched": "<img src=x onerror=alert(2)>", "left_only": 0, "right_only": 0, "exceptions": 1},
        "findings": [{
            "key": "<script>alert(1)</script>",
            "left_value": "ACTIVE",
            "right_value": "INACTIVE",
            "left_row": 3,
            "right_row": 3,
            "status": "FAIL",
        }],
    }],
}

with tempfile.TemporaryDirectory(prefix="migraven report ") as directory:
    root = pathlib.Path(directory)
    source = root / "result.json"
    target = root / "report.html"
    source.write_text(json.dumps(REPORT), encoding="utf-8")
    result = subprocess.run(
        [sys.executable, str(SCRIPT), str(source), "--output", str(target)],
        text=True, capture_output=True, check=False,
    )
    assert result.returncode == 0, result.stderr
    html = target.read_text(encoding="utf-8")
    assert "Migration verification report" in html
    assert "INACTIVE" in html
    assert "legacy.csv" in html
    assert "new.csv" in html
    assert "18 rows" in html
    assert "&lt;script&gt;alert(1)&lt;/script&gt;" in html
    assert "<script>alert(1)</script>" not in html
    assert "&lt;img src=x onerror=alert(2)&gt;" in html
    assert "<img src=x onerror=alert(2)>" not in html

    invalid = root / "invalid.json"
    invalid.write_text('{"status":"BROKEN"}', encoding="utf-8")
    rejected = subprocess.run(
        [sys.executable, str(SCRIPT), str(invalid)],
        text=True, capture_output=True, check=False,
    )
    assert rejected.returncode == 2

    malformed = root / "malformed.json"
    malformed.write_text(
        json.dumps({"status": "PASS", "checks": [{
            "field": "amount", "summary": [], "findings": []
        }]}),
        encoding="utf-8",
    )
    malformed_result = subprocess.run(
        [sys.executable, str(SCRIPT), str(malformed)],
        text=True, capture_output=True, check=False,
    )
    assert malformed_result.returncode == 2
    assert "Invalid summary object" in malformed_result.stderr

print("all report renderer tests passed")
