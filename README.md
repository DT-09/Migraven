# VERITY

**Migration export verification with source-row evidence.**

Verity is a local-first C17 utility for implementation consultancies and migration teams. It compares agreed source and target CSV or flat-object JSONL exports by a unique key, checks selected fields using exact string equality, and produces reviewable findings with source-row references.

> **Product status:** early evaluation product. It is not production-certified, independently security-assessed, or a migration-correctness guarantee. No customer, savings, or performance claims are made.

## Who it is for

ERP/CRM implementation consultancies and migration delivery teams that need a repeatable check on selected records before internal or client sign-off.

**The job:** compare two agreed exports and give the migration lead a traceable exception list to review.

Verity is deliberately narrower than a full migration-testing platform. It does not replace transformation testing, schema mapping, integration testing, or business acceptance.

## What it does

- Reads CSV and flat-object JSONL locally.
- Accepts a UTF-8 byte-order mark at the start of CSV exports and rejects embedded NUL bytes.
- Validates headers and row widths.
- Requires a non-empty, unique comparison key in each input.
- Compares up to 32 selected fields per run using exact string equality.
- Reports unequal values and keys present on only one side.
- Preserves input-row references in JSON findings.
- Produces a self-contained HTML report for human review.
- Creates a SHA-256 manifest binding the two input files and generated JSON report.

A finding is an observation, not proof of a migration defect. A PASS applies only to the selected key and fields.

## Build and test

Requirements: CMake 3.16+, a C17 compiler, and Python 3.10+.

```bash
cmake -S . -B build -DVERITY_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

The test suite covers the C core, CLI behavior, CSV edge cases, HTML escaping, evidence-manifest integrity, the end-to-end demo and release packaging. CI builds and tests with GCC, Clang and MSVC across Linux, macOS and Windows.

## Run the complete demonstration

```bash
python tools/demo.py
```

The script uses bundled synthetic records and writes `verity-demo-output/` with:

| File | Purpose |
|---|---|
| `comparison.json` | Machine-readable field results |
| `review.html` | Self-contained report for a human reviewer |
| `evidence.json` | SHA-256 manifest for the inputs and JSON report |

It then verifies the manifest and prints the output paths. No customer data is used.

If the executable is outside the default build location, pass it explicitly:

```bash
python tools/demo.py --binary build/Release/verity.exe
```

## Direct comparison

```bash
./build/verity inspect examples/migration_source.csv

./build/verity compare \
  --key record_id \
  --field account_status \
  examples/migration_source.csv \
  examples/migration_target.csv
```

For JSONL, specify the format:

```bash
./build/verity compare --format jsonl \
  --key record_id --field account_status \
  examples/migration_source.jsonl examples/migration_target.jsonl
```

JSONL records must be flat objects with matching property names; property order may differ. String, number, boolean and null values are accepted. Nested arrays and objects are not supported.

To render a saved JSON result as HTML:

```bash
./build/verity compare --key record_id --field account_status \
  examples/migration_source.csv examples/migration_target.csv > result.json

python tools/render_report.py result.json --output review.html
```

To create and verify a SHA-256 evidence manifest:

```bash
python tools/evidence.py create \
  --left examples/migration_source.csv \
  --right examples/migration_target.csv \
  --report result.json \
  --key record_id --field account_status \
  --format csv --tool-version 0.1.0 \
  --output evidence.json

python tools/evidence.py verify evidence.json
```

**Manifest trust boundary:** the manifest is unsigned. It detects changes relative to the recorded hashes and sizes, but does not prove who created it or prevent replacement of both the manifest and files. It is an integrity aid, not a digital signature or audit certification. When files are on separate Windows drives, absolute paths are recorded; those manifests are verifiable on the originating host but are not portable.

## Build a local release bundle

After building Verity, create a ZIP package containing the executable, documentation, synthetic examples, demo tools and generated demo evidence:

```bash
python tools/package_release.py \
  --binary build/verity \
  --version 0.1.0 \
  --platform local \
  --output-dir dist
```

On Windows, use `--binary build/Release/verity.exe`. The packager creates a ZIP and a matching SHA-256 checksum file. Tagged releases are built and tested by the GitHub Actions release workflow.

## Output contract

The CLI emits one JSON object. Top-level status is `PASS` only when all selected fields have no findings; otherwise it is `REVIEW`. Each entry in `checks` includes the field name, counts and findings. A paired record with unequal values is `FAIL`; a key present on only one side is `INDETERMINATE`.

## Current limits

- CSV and flat-object JSONL only; no database connectors.
- Up to 32 fields per run; fields are compared independently.
- Exact string equality only; no numeric tolerance, date normalization, currency conversion, fuzzy matching or business-rule inference.
- No schema mapping, relationship validation, aggregate reconciliation or transformation validation.
- Key matching uses sorted indexes: O((n + m) log(n + m)) lookup preparation and O(n + m) auxiliary memory per selected field. Benchmark representative exports before operational use.
- SHA-256 manifests are unsigned and do not establish authenticity.
- No independent security assessment, compliance certification or production approval.
- No claim that a PASS proves overall migration correctness.

## Paid evaluation

The initial offer is a bounded **Migration Export Verification Evaluation**:

- one source/target dataset pair;
- one agreed unique key;
- up to three critical fields;
- CSV or flat-object JSONL;
- JSON result, HTML review report, exception register and SHA-256 manifest;
- a review note separating observed differences from unresolved explanations.

See:
- [Buyer one-pager](docs/buyer-one-pager.md)
- [Product brief](docs/product-brief.md)
- [First market and buyer qualification](docs/first-market.md)
- [Paid pilot scope and acceptance criteria](docs/paid-pilot.md)
- [Data handling and evaluation boundaries](docs/data-handling.md)
- [Paid evaluation proposal template](docs/pilot-proposal-template.md)

The evaluation is a proposed commercial offer, not a proven price or established customer engagement. Fee, delivery date, payment route, data-handling terms and any license grant must be agreed before work starts.

## Licensing

No open-source or commercial license has been selected. Do not redistribute, deploy for a customer, or assume commercial-use rights until written license terms are agreed. Normal copyright restrictions apply.
