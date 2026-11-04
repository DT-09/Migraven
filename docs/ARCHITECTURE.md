# Architecture

## Product boundary

Migraven is a local-first verification engine for comparing agreed source and target exports before migration sign-off. It is not a migration tool, data-cleaning platform, ETL engine, or proof that an entire migration is correct.

## Current implementation

migraven_core is a dependency-free C17 static library. The CLI consumes the public API in include/migraven/migraven.h.

- csv.c: bounded CSV loading, quoted-field parsing, row-width validation and table ownership.
- jsonl.c: bounded flat-object JSONL parsing, scalar normalization, UTF-8 preservation, duplicate-property rejection and schema consistency checks.
- compare.c: unique-key validation, sorted key indexes, exact field comparison and source-row findings.
- util.c: status labels and JSON string serialization.
- main.c: CLI argument parsing and machine-readable report output.
- tools/render_report.py: local, self-contained HTML evidence report rendering with escaped dynamic values.

## Data model

Both input adapters populate the same table representation: ordered column names, string-valued cells and physical source line numbers.

JSONL strings are decoded. Number and boolean tokens are retained as their lexical representation. JSON null becomes an empty string. Nested arrays and objects are rejected. JSONL records must use the same property names; property order may differ.

The current model is intentionally conservative. It does not infer numeric, date, currency, timezone or domain-specific types. 10, 10.0 and "10" remain different values.

## Comparison semantics

The caller selects one unique key column and one or more fields. Keys must be non-empty and unique in both datasets. Matching is exact. Each selected field produces an independent result.

- PASS: a paired key has equal values for the selected field.
- FAIL: a paired key has unequal values.
- INDETERMINATE: a key exists on only one side; the engine does not infer why.

For each field comparison, sorted key indexes provide O((n + m) log(n + m)) index preparation and O(n + m) auxiliary memory. Multiple fields are evaluated independently. Output findings retain original input row numbers. Benchmark representative datasets before making throughput claims.

## Trust and evidence boundary

A report records observations made from the supplied files under explicit comparison rules. It does not establish that the source exports were complete, authentic, correctly extracted, or semantically equivalent. The `tools/evidence.py` utility creates a versioned SHA-256 manifest binding both input files and the JSON report, along with format, key, fields, semantics, tool version and timestamp. The verifier re-hashes all three files and checks byte sizes. The manifest is unsigned: it detects changes relative to the recorded manifest but does not establish author identity or protect against replacement of the manifest itself. Trusted storage or a digital signature is required for stronger provenance.

## Product website

The static website in `website/` is a separate presentation layer. It describes only implemented CLI behavior and documented evaluation boundaries. It does not process customer files or imply that the CLI is a hosted service. Cloudflare Pages can serve the directory as a static site with no build step.

## Remaining engineering gates

1. Define explicit typed-value and normalization policies only when a buyer requires them; do not silently alter exact-string semantics.
2. Add representative large-file, memory-limit and performance tests before publishing throughput guidance.
3. Add fuzzing and sanitizer coverage for CSV, JSONL and report parsing.
4. Evaluate signed manifests and deterministic report serialization if stronger chain-of-custody requirements emerge.
5. Complete independent security and data-handling review before making production-assurance claims.
