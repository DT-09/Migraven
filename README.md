# VERITY

**Independent migration verification for CSV exports.**

Verity is a local-first C17 utility for checking agreed source and target data exports before migration sign-off. It matches records by a unique key, compares a selected field, and returns machine-readable findings with source-row references.

> Status: early MVP. Not production-certified. No performance, security-certification, or customer claims are made.

## First use case

A software implementation consultancy has exported records from a legacy system and the migrated system. Before sign-off, the migration lead needs to check whether agreed critical values survived the move.

Verity provides a narrow, reproducible check:
- Read source and target CSV files locally.
- Validate headers and row widths.
- Require a non-empty, unique comparison key in each file.
- Compare one agreed field per run using exact string equality.
- Report mismatched values and keys present on only one side.
- Preserve source row references in JSON output.

Run one comparison per critical field. Keep the resulting JSON files together as the evidence bundle. See [first-market positioning](docs/first-market.md) and the [paid pilot scope](docs/paid-pilot.md).

## Quick start

Requirements: CMake 3.16+ and a C17 compiler.

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

The synthetic example intentionally contains:
- one matched key with a changed status;
- one key present only in the source;
- one key present only in the target.

Expected report summary: 4 matched keys, 1 source-only key, 1 target-only key, 3 findings. A finding is an observation for human review, not proof of a migration defect.

## Current limits

- CSV only; no direct database connectors.
- One selected field per comparison run.
- Exact string equality; no numeric tolerance, date normalization, currency conversion, fuzzy matching or business-rule inference.
- No schema mapping, relationship validation, aggregate reconciliation or transformation validation.
- No cryptographic evidence manifest or tamper-evident audit trail.
- No claim that a PASS proves overall migration correctness.
- Not certified for regulated or production-critical validation.

## Design boundaries

- Local-first execution.
- Stable input-order results.
- Source-row traceability.
- Conservative handling of malformed input and duplicate keys.
- A passing rule proves only that the specified comparison passed.

## License

No license has been selected. Normal copyright restrictions apply; do not assume open-source or commercial reuse rights.
