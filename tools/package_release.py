#!/usr/bin/env python3
"""Assemble a tested, self-contained Verity release bundle and SHA-256 checksum."""
from __future__ import annotations
import argparse, hashlib, shutil, subprocess, sys, zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--platform", required=True)
    parser.add_argument("--output-dir", default="dist")
    args = parser.parse_args()
    binary = Path(args.binary).resolve()
    if not binary.is_file():
        parser.error(f"compiled executable does not exist: {binary}")
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    name = f"Verity-{args.version}-{args.platform}"
    stage = output_dir / name
    if stage.exists():
        shutil.rmtree(stage)
    (stage / "bin").mkdir(parents=True)
    executable_name = "verity.exe" if binary.suffix.lower() == ".exe" else "verity"
    shutil.copy2(binary, stage / "bin" / executable_name)
    shutil.copy2(ROOT / "README.md", stage / "README.md")
    for folder in ("docs", "examples"):
        shutil.copytree(ROOT / folder, stage / folder)
    (stage / "tools").mkdir()
    for filename in ("demo.py", "render_report.py", "evidence.py"):
        shutil.copy2(ROOT / "tools" / filename, stage / "tools" / filename)
    demo_output = stage / "demo-output"
    subprocess.run([
        sys.executable, str(ROOT / "tools" / "demo.py"),
        "--binary", str(stage / "bin" / executable_name),
        "--examples-dir", str(stage / "examples"),
        "--output", str(demo_output),
    ], cwd=ROOT, check=True)
    archive = output_dir / f"{name}.zip"
    if archive.exists():
        archive.unlink()
    with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as bundle:
        for path in sorted(stage.rglob("*")):
            if path.is_file():
                bundle.write(path, path.relative_to(output_dir))
    checksum = output_dir / f"{archive.name}.sha256"
    checksum.write_text(f"{sha256(archive)}  {archive.name}\n", encoding="ascii")
    print(f"PACKAGE: {archive}")
    print(f"SHA256: {checksum}")
    print(f"DEMO: {demo_output}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
