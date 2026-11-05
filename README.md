# Migraven

**Migration export verification with source-row evidence.**

Migraven is a local-first migration verification product for ERP/CRM implementation consultancies, migration delivery teams and data-assurance reviewers. Its dependency-free C17 engine compares agreed source and target CSV or flat-object JSONL exports by a unique key, checks selected fields using exact string equality, and produces traceable findings.

> **Product status:** evaluation-stage software. It is not independently security-assessed, certified, or a guarantee of migration correctness. No customer, savings, throughput or defect-detection claims are made.

## Product in one sentence

Turn two agreed migration exports into a field-specific, row-linked exception register that a human reviewer can investigate.

## Core architecture

- **C17 core and CLI:** bounded CSV/JSONL parsing, structural validation, unique-key validation, sorted indexes, deterministic merge-join comparison and JSON output.
- **Python 3.10+ utilities:** local demonstration orchestration, evidence-manifest generation and verification, offline HTML report rendering and release packaging.
- **Static browser application:** an interactive CSV demonstration that runs locally in the visitor's browser. It does not upload or persist selected files.
- **Website:** static multi-page product presentation; no hosted customer-data service or payment processing is implied.

C++ is deliberately not added as a second native engine. The comparison contract stays in one dependency-free C17 implementation. Python is used for orchestration and evidence packaging rather than duplicating the core algorithm. The website demo is a browser-side demonstration of the same documented exact-match rules; the production CLI remains the C17 implementation.

## Who it is for

- ERP/CRM implementation consultancies running repeat migration projects.
- Internal migration delivery teams preparing checkpoint reviews.
- Data assurance reviewers who need row-level context for exceptions.

## What it does

- Reads CSV and flat-object JSONL locally.
- Validates headers, row widths and input structure.
- Requires a non-empty, unique comparison key in each dataset.
- Compares up to 32 selected fields independently using exact string equality.
- Identifies unequal values and keys present on only one side.
- Preserves original source-row references.
- Produces machine-readable JSON, a self-contained HTML report and a SHA-256 manifest.

A finding is an observation, not proof of a migration defect. PASS applies only to the selected key and fields.

## Build and test

Requirements: CMake 3.16+, a C17 compiler, and Python 3.10+.

```bash
cmake -S . -B build -DMIGRAVEN_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

The suite covers the C core, CLI, CSV edge cases, HTML escaping, evidence integrity, the end-to-end demonstration, release packaging and website link/navigation smoke tests.

## Run the bundled demonstration

```bash
python tools/demo.py
```

The script uses synthetic records and writes `migraven-demo-output/` containing:
- `comparison.json` — machine-readable field results;
- `review.html` — self-contained human-review report;
- `evidence.json` — SHA-256 manifest for both inputs and the JSON report.

## Direct comparison

```bash
./build/migraven inspect examples/migration_source.csv

./build/migraven compare \
  --key record_id \
  --field account_status \
  --field account_name \
  examples/migration_source.csv examples/migration_target.csv
```

For JSONL, specify `--format jsonl`. JSONL records must be flat objects with matching property names; property order may differ. String, number, boolean and null scalar values are supported. Nested arrays and objects are not supported.

## Comparison algorithm

For each selected field, the engine:
1. validates that keys are non-empty and unique on both sides;
2. builds sorted key indexes that retain each record's original row index;
3. performs a two-pointer merge join over those indexes;
4. emits value-difference and one-sided-key findings in key order.

Index construction is (O(n log n + m log m)); the merge pass is (O(n+m)). Auxiliary key-index memory is (O(n+m)), excluding parsed tables and findings. The CLI currently repeats the index construction for each selected field; a shared multi-field index is a measured optimisation candidate, not a current claim.

## Evidence boundary

The evidence utility records SHA-256 hashes and byte sizes for both input files and the generated JSON report. The manifest is unsigned. It detects changes relative to the recorded manifest but does not prove who created it, authenticate the original exports, or prevent replacement of both the manifest and files. It is an integrity aid, not a digital signature or audit certification.

## Product website and browser demo

The multi-page static website is in [website/](website/). It includes an interactive CSV demonstration at [website/demo.html](website/demo.html). The demo parses selected files in the browser tab and does not upload, save or transmit them. It demonstrates the documented exact-key/exact-value workflow; the production comparison engine remains the C17 CLI/library.

For Cloudflare Pages, set the output directory to `website` and leave the build command empty.

## Commercial path

The proposed route is:
1. **Paid evaluation:** one source/target pair, one key, up to three critical fields and an agreed evidence package.
2. **Team licence:** recurring annual use after licence, support, update and data terms are finalised.
3. **Consultancy partner licence:** repeat project use across a delivery partner's portfolio.

These are proposed stages, not active payment plans. No validated public price, checkout, commercial licence or customer data-processing agreement is currently represented as live. Fee, delivery, payment route, data handling, authorisation and licence terms must be agreed before work starts. Start with synthetic or properly de-identified data.

See [docs/ENGINE_CONTRACT.md](docs/ENGINE_CONTRACT.md), [docs/COMMERCIAL_MODEL.md](docs/COMMERCIAL_MODEL.md), [docs/data-handling.md](docs/data-handling.md), [docs/paid-pilot.md](docs/paid-pilot.md) and [docs/first-market.md](docs/first-market.md).

## Licensing

No open-source or commercial licence has been selected. Do not redistribute or deploy for a customer until written licence terms are agreed.
