# Customer quick start

This guide assumes you received a Verity platform ZIP and have Python 3.10+ installed. Verity does not upload your files; run it on a machine you control.

## 1. Extract the package

Extract the ZIP. The folder contains:

- `bin/verity` or `bin/verity.exe` — comparison engine.
- `examples/` — synthetic sample exports.
- `tools/` — local HTML report and evidence utilities.
- `demo-output/` — completed synthetic example, including JSON, HTML and manifest.
- `docs/` — product scope, CLI and data-handling notes.

Run the bundled demonstration first. It uses synthetic data only.

### Windows PowerShell

```powershell
python .\tools\demo.py --binary .\bin\verity.exe --examples-dir .\examples --output .\demo-output-check
```

### macOS / Linux

```bash
python3 tools/demo.py --binary ./bin/verity --examples-dir ./examples --output ./demo-output-check
```

## 2. Prepare the two exports

Place the source and target CSV files in a controlled working directory. Confirm that:

- both exports use the same unique record key;
- the key is non-empty and unique in each file;
- the selected fields exist in both exports;
- the selected values should be compared as exact strings;
- you are authorized to process the data.

Start with synthetic or de-identified files. Do not use production credentials or send confidential files through an informal channel.

## 3. Run a comparison

Example with a key called `record_id` and a field called `account_status`.

Windows PowerShell:

```powershell
.\bin\verity.exe compare --key record_id --field account_status C:\work\source.csv C:\work\target.csv > C:\work\comparison.json
```

macOS / Linux:

```bash
./bin/verity compare --key record_id --field account_status /work/source.csv /work/target.csv > /work/comparison.json
```

Repeat `--field` for additional columns, up to 32 distinct fields. Each field is evaluated independently. Verity uses exact string equality; it does not normalize dates, numbers, currencies, whitespace or case.

A process exit code of zero means the comparison ran. It does not mean the datasets matched. Inspect the JSON `status` value:

- `PASS`: no findings for the selected key and fields.
- `REVIEW`: one or more differences or one-sided keys require review.

## 4. Render the review report

Run the renderer with Python 3.10+:

```bash
python tools/render_report.py /work/comparison.json --output /work/review.html
```

Open `review.html` locally. It is self-contained. Treat it as sensitive because it may contain source values, target values and record keys.

## 5. Create and verify the evidence manifest

```bash
python tools/evidence.py create \
  --left /work/source.csv \
  --right /work/target.csv \
  --report /work/comparison.json \
  --key record_id \
  --field account_status \
  --format csv \
  --tool-version 0.1.0 \
  --output /work/evidence.json

python tools/evidence.py verify /work/evidence.json
```

The manifest binds the two input files and the JSON report by SHA-256 hash and byte size. Keep the manifest in a trusted location separate from the files it describes if you need stronger protection against replacement. The current manifest is unsigned; `VERIFIED` confirms hash/size agreement with that manifest, not who created it.

## 6. Review before sign-off

A finding is an observation, not proof of a migration defect. Check the relevant source and target records, extraction completeness, transformation rules and business context. A PASS applies only to the configured key and selected fields. It does not certify the full migration.
