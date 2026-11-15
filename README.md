# Migraven

**Migration export verification with source-row evidence.**

Migraven compares agreed source and target exports by a unique key, checks operator-selected fields using exact string equality, and produces a traceable exception register for human review.

> **Status: evaluation-stage software.** The repository contains a working C17 engine, local CLI, evidence utilities, synthetic demonstration and static website. It is not independently security-assessed or certified. No customer, savings, throughput or defect-detection claims are made. No commercial licence, public price, checkout or customer data-processing agreement is currently live.

## What Migraven does

- Reads CSV and flat-object JSONL exports locally.
- Validates headers, row structure and non-empty unique keys.
- Compares up to 32 selected fields independently.
- Uses exact key and exact string equality; it does not normalize values.
- Identifies unequal values and keys present on only one side.
- Retains original source-row references.
- Produces machine-readable JSON, a self-contained HTML review report and an unsigned SHA-256 evidence manifest.

A finding is an observation, not proof of a migration defect. PASS applies only to the selected key and fields. It does not certify the migration or prove business correctness.

## Architecture

| Layer | Implementation | Responsibility |
|---|---|---|
| Comparison engine | Dependency-free C17 | CSV/JSONL parsing, key validation, sorted indexes and deterministic merge join |
| CLI | C17 | Inspect exports and compare selected fields; JSON output |
| Evidence utilities | Python 3.10+ standard library | HTML report rendering, SHA-256 manifest creation/verification and release packaging |
| Browser demo | Static HTML/CSS/JavaScript | Synthetic or user-selected CSV comparison in the current browser tab |
| Website | Static multi-page site | Product explanation, method, limitations, demo and evaluation enquiry draft |

The browser demo is illustrative. It does not upload or persist selected files. The production comparison engine is the C17 CLI/library; the browser implementation is not a substitute for it.

## Build and test

Requirements: CMake 3.16+, a C17 compiler, and Python 3.10+.

~~~bash
cmake -S . -B build -DMIGRAVEN_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --config Release --output-on-failure
~~~

The test suite covers the native core, CLI behavior, CSV edge cases, HTML escaping, evidence integrity, the end-to-end demonstration, release packaging and website navigation/assets. GitHub Actions runs GCC and Clang builds, cross-platform package validation, website checks and sanitizer checks.

## Run the bundled demonstration

~~~bash
python tools/demo.py
~~~

The script uses synthetic records and writes migraven-demo-output/ containing:

- comparison.json — machine-readable field results;
- review.html — self-contained human-review report;
- evidence.json — SHA-256 manifest for both inputs and the JSON report.

## Direct comparison

~~~bash
./build/migraven inspect examples/migration_source.csv

./build/migraven compare \
  --key record_id \
  --field account_status \
  --field account_name \
  examples/migration_source.csv examples/migration_target.csv
~~~

For JSONL, add --format jsonl. JSONL records must be flat objects with matching property names. String, number, boolean and null scalar values are supported; nested arrays and objects are outside the current contract.

Use --strict to return exit code 1 when findings exist. Input or execution errors return exit code 2.

## Comparison contract

1. The operator supplies one key and one or more fields.
2. The key must exist and be non-empty and unique in both datasets.
3. Keys are sorted while retaining original source-row references.
4. A two-pointer merge join pairs records independently of input row order.
5. Each selected field is compared using exact string equality.
6. Mismatches and one-sided keys are emitted in key order.

Index construction is O(n log n + m log m); the merge pass is O(n+m). Auxiliary key-index memory is O(n+m), excluding parsed tables and findings. The current CLI rebuilds indexes for each selected field; shared multi-field indexing remains a future measured optimization.

## Evidence and data handling

The evidence utility hashes the source file, target file and generated JSON report. The manifest is unsigned. It detects changes relative to the recorded manifest but does not prove authorship, authenticate original exports, or prevent replacement of both files and manifest.

Comparison outputs can contain keys and values from both datasets. Treat reports and manifests as sensitive whenever their inputs are sensitive. Initial evaluations should use synthetic or properly de-identified exports. Do not provide production credentials, database dumps or confidential customer records through informal channels.

See [data handling](docs/data-handling.md), [engine contract](docs/ENGINE_CONTRACT.md) and [release readiness](docs/RELEASE_CHECKLIST.md).

## Website

The static product site is in [website/](website/). It includes a browser-side CSV demonstration at [website/demo.html](website/demo.html).

For Cloudflare Pages, set the output directory to website and leave the build command empty. See [website deployment notes](website/README.md).

## Commercial path

The proposed sequence is a bounded paid evaluation, followed by a team licence only if repeat use is demonstrated, and a consultancy partner licence where multi-project rights and support are defined. These are proposed packages, not active payment plans.

Scope, fee, delivery, authorization, data handling, payment route and licence terms must be agreed in writing before work starts. No fixed price is represented as validated market pricing. No open-source or commercial licence has been selected; do not redistribute or deploy for a customer until written licence terms are agreed.

## Contributing and security

- Read [CONTRIBUTING.md](CONTRIBUTING.md) before proposing changes.
- See [SECURITY.md](SECURITY.md) for responsible vulnerability reporting.
- Review [docs/RELEASE_CHECKLIST.md](docs/RELEASE_CHECKLIST.md) before publishing a release.
