import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile

SCRIPT = pathlib.Path(__file__).resolve().parents[1] / "tools" / "evidence.py"


def run(*args):
    return subprocess.run(
        [sys.executable, str(SCRIPT), *map(str, args)],
        text=True, capture_output=True, check=False,
    )


with tempfile.TemporaryDirectory(prefix="migraven evidence ") as directory:
    root = pathlib.Path(directory)
    left = root / "source.csv"
    right = root / "target.csv"
    report = root / "result.json"
    manifest = root / "evidence.json"
    left.write_text("id,value\n1,A\n", encoding="utf-8")
    right.write_text("id,value\n1,B\n", encoding="utf-8")
    report.write_text(json.dumps({
        "status": "REVIEW", "key": "id", "format": "csv",
        "checks": [{"field": "value", "summary": {}, "findings": []}],
    }) + "\n", encoding="utf-8")

    created = run(
        "create", "--left", left, "--right", right, "--report", report,
        "--key", "id", "--field", "value", "--format", "csv",
        "--tool-version", "0.1.0", "--output", manifest,
    )
    assert created.returncode == 0, created.stderr
    data = json.loads(manifest.read_text(encoding="utf-8"))
    assert data["schema"] == "migraven-evidence/v1"
    assert data["comparison"]["key"] == "id"
    assert data["comparison"]["fields"] == ["value"]
    assert data["files"]["left"]["sha256"] == hashlib.sha256(left.read_bytes()).hexdigest()
    assert set(data["files"]) == {"left", "right", "report"}

    valid = run("verify", manifest)
    assert valid.returncode == 0, valid.stdout + valid.stderr
    assert json.loads(valid.stdout)["status"] == "VERIFIED"

    report.write_text(json.dumps({
        "status": "REVIEW", "key": "id", "format": "csv",
        "checks": [{"field": "value", "summary": {}, "findings": []}],
        "extra": "changed",
    }) + "\n", encoding="utf-8")
    changed = run("verify", manifest)
    assert changed.returncode == 1
    assert json.loads(changed.stdout)["status"] == "TAMPERED_OR_MISSING"
    assert any(x["file"] == "report" and x["status"] == "FAIL"
               for x in json.loads(changed.stdout)["checks"])

    mismatch = run(
        "create", "--left", left, "--right", right, "--report", report,
        "--key", "wrong", "--field", "value", "--format", "csv",
        "--tool-version", "0.1.0", "--output", root / "mismatch.json",
    )
    assert mismatch.returncode == 2
    collision = run(
        "create", "--left", left, "--right", right, "--report", report,
        "--key", "id", "--field", "value", "--format", "csv",
        "--tool-version", "0.1.0", "--output", report,
    )
    assert collision.returncode == 2

    report.unlink()
    missing = run("verify", manifest)
    assert missing.returncode == 1
    assert any(x["file"] == "report" and x["status"] == "MISSING"
               for x in json.loads(missing.stdout)["checks"])

print("all evidence manifest tests passed")
