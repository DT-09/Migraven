# VERITY

**Independent migration verification for CSV and JSONL exports.**

Verity is a local-first C17 utility for checking agreed source and target CSV or JSONL exports before migration sign-off. It matches records by a unique key, checks several selected fields in one run, and returns machine-readable findings with source-row references.

> Status: early MVP. Not production-certified. No performance, security-certification, or customer claims are made.

## First use case

A software implementation consultancy has exported records from a legacy system and the migrated system. Before sign-off, the migration lead needs to check whether agreed critical values survived the move.

Verity provides a narrow, reproducible check:
- Read source and target CSV or JSONL files locally.
- Validate headers and row widths.
- Require a non-empty, unique comparison key in each file.
- Compare up to 32 selected fields in one invocation using exact string equality.
- Report mismatched values and keys present on only one side for each field.
- Preserve source row references in JSON output.
- Read each input file once per invocation; each field is evaluated independently.

See [first-market positioning](docs/first-market.md) and the [paid pilot scope](docs/paid-pilot.md).

## Quick start

Requirements: CMake 3.16+, a C17 compiler, and Python 3 when tests are enabled.

```bash
cmake -S . -B build -DVERITY_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Demonstration

```bash
./build/verity inspect examples/migration_source.csv

./build/verity compare \
  --key record_id \
  --field account_status \
  examples/migration_source.csv \
  examples/migration_target.csv
```

For JSONL, select the input format explicitly with `--format jsonl` (the default is CSV):

```bash
./build/verity compare --format jsonl \
  --key record_id --field account_status \
  examples/migration_source.jsonl examples/migration_target.jsonl
```

JSONL records must be objects with the same property names; property order may differ. String, number, boolean and null values are accepted; nested arrays and objects are not.

Check multiple fields in one pass by repeating `--field`:

```bash
./build/verity compare \
  --key record_id \
  --field account_status \
  --field region \
  --field credit_limit \
  source.csv target.csv
```

The synthetic example intentionally contains:
- one matched key with a changed status;
- one key present only in the source;
- one key present only in the target.

Expected report summary for the supplied example: 4 matched keys, 1 source-only key, 1 target-only key, 3 findings. A finding is an observation for human review, not proof of a migration defect.

## Human-readable evidence report

Save the comparison JSON, then render a self-contained HTML report for review:

```bash
./build/verity compare --key record_id --field account_status \
  examples/migration_source.csv examples/migration_target.csv > result.json

python tools/render_report.py result.json --output review.html
```

The HTML report is generated locally. It includes per-field counts, findings, source-row references and a clear human-review disclaimer. It uses no external scripts, fonts or network services.

## Cryptographic evidence manifest

After creating the JSON report, bind the two input files and report to a versioned SHA-256 manifest:

```bash
python tools/evidence.py create \\
  --left examples/migration_source.csv \\
  --right examples/migration_target.csv \\
  --report result.json \\
  --key record_id --field account_status \\
  --format csv --tool-version 0.1.0 \\
  --output evidence.json

python tools/evidence.py verify evidence.json
```

The verifier re-hashes both inputs and the report and checks their recorded byte sizes. It returns `VERIFIED` only when all three match. A changed or missing file produces a non-zero exit code. The manifest records the format, key, fields, comparison semantics and tool version.

**Trust boundary:** the manifest is unsigned. It detects changes relative to the manifest but does not prove who created the manifest or prevent an attacker from replacing both the files and manifest. Preserve the manifest in a trusted, access-controlled location or add a trusted digital signature before using it as formal audit evidence.

## Output contract

The command emits one JSON object. The top-level status is `PASS` only when every selected field has no findings; otherwise it is `REVIEW`. Each entry in `checks` contains its field name, match/exception counts and field-specific findings. A missing record is `INDETERMINATE`; a paired record with unequal values is `FAIL`.

## Current limits

- CSV and JSONL only; no direct database connectors.
- Up to 32 fields per run; fields are compared independently.
- Exact string equality; no numeric tolerance, date normalization, currency conversion, fuzzy matching or business-rule inference.
- No schema mapping, relationship validation, aggregate reconciliation or transformation validation.
- For each selected field, key matching uses sorted indexes with O((n + m) log(n + m)) lookup preparation and O(n + m) auxiliary memory; benchmark representative exports before production use.
- SHA-256 manifests detect changes to recorded inputs and reports, but manifests are unsigned and do not establish authenticity or provide a tamper-evident audit trail.
- No claim that a PASS proves overall migration correctness.
- Not certified for regulated or production-critical validation.

## Design boundaries

- Local-first execution.
- Stable input-order results.
- Source-row traceability.
- Conservative handling of malformed input and duplicate keys.
- A passing rule proves only that the specified comparison passed.

## Commercial and distribution status

Verity is an early evaluation product. No open-source or commercial license has been selected. Do not redistribute, deploy for a customer, or assume commercial-use rights until written license terms are agreed. See [the evaluation scope](docs/paid-pilot.md) and [data-handling boundaries](docs/data-handling.md).
