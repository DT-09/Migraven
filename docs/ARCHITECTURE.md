# Architecture

## Product boundary

Verity is a local-first verification engine for comparing agreed source and target exports before migration sign-off. It is not a migration tool, data-cleaning platform, ETL engine, or proof that an entire migration is correct.

## Current implementation

verity_core is a dependency-free C17 static library. The CLI consumes the public API in include/verity/verity.h.

- csv.c: bounded CSV loading, quoted-field parsing, row-width validation and table ownership.
- jsonl.c: bounded flat-object JSONL parsing, scalar normalization, UTF-8 preservation, duplicate-property rejection and schema consistency checks.
- compare.c: unique-key validation, sorted key indexes, exact field comparison and source-row findings.
- util.c: status labels and JSON string serialization.
- main.c: CLI argument parsing and machine-readable report output.

## Data model

Both input adapters populate the same table representation: ordered column names, string-valued cells and physical source line numbers.

JSONL strings are decoded. Number and boolean tokens are retained as their lexical representation. JSON null becomes an empty string. Nested arrays and objects are rejected. JSONL records must use the same property names; property order may differ.

The current model is intentionally conservative. It does not infer numeric, date, currency, timezone or domain-specific types. 10, 10.0 and "10" remain different values.

## Comparison semantics

The caller selects one unique key column and one or more fields. Keys must be non-empty and unique in both datasets. Matching is exact. Each selected field produces an independent result.

- PASS: a paired key has equal values for the selected field.
- FAIL: a paired key has unequal values.
- INDETERMINATE: a key exists on only one side; the engine does not infer why.

For each field comparison, sorted key indexes provide O((n + m) log(n + m)) index preparation and O(n + m) auxiliary memory. Multiple fields are evaluated independently; benchmark representative datasets before making throughput claims. Output findings retain original input row numbers. Benchmark representative datasets before making throughput claims.

## Trust and evidence boundary

A report records observations made from the supplied files under explicit comparison rules. It does not establish that the source exports were complete, authentic, correctly extracted, or semantically equivalent. A future evidence-manifest layer should bind input hashes, tool version, rule configuration, timestamps and output digest. That layer is not implemented yet.

## Next engineering gates

1. Typed canonical records with explicit type and normalization policies.
2. Versioned rule registry and schema mapping.
3. SHA-256 input/output evidence manifest and deterministic report serialization.
4. Python bindings and stable shared-library ABI.
5. Fuzzing, sanitizer runs, memory/large-file tests and benchmark suite.
6. Independent review of security and data-handling claims before production use.
