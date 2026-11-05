# Architecture

## Product boundary

Migraven is a local-first verification engine for comparing agreed source and target exports before a migration checkpoint. It is not a migration tool, ETL engine, schema mapper, data-cleaning platform or proof that an entire migration is correct.

## Language and runtime boundaries

- **C17:** canonical parsing, key validation, comparison algorithm, public library API and CLI.
- **Python 3.10+:** orchestration, evidence manifest generation/verification, offline report rendering and release packaging.
- **Browser JavaScript:** interactive CSV demonstration only. It mirrors the documented exact-match semantics and is not the production C engine.
- **TypeScript / C++:** intentionally not introduced yet. They would add build and maintenance surface without a current product requirement. A future hosted workspace or typed client SDK should justify TypeScript; the core remains C17.

## C17 engine

- `csv.c`: bounded CSV loading, quoted-field parsing, row-width validation and table ownership.
- `jsonl.c`: bounded flat-object JSONL parsing, scalar normalization, UTF-8 preservation, duplicate-property rejection and schema consistency checks.
- `compare.c`: unique-key validation, sorted key indexes, deterministic two-pointer merge join, exact field comparison and original source-row findings.
- `util.c`: status labels and JSON string serialization.
- `main.c`: CLI argument parsing and machine-readable report output.

Both adapters populate a shared table representation: ordered column names, string-valued cells and physical source line numbers. JSONL strings are decoded. Number and boolean tokens retain lexical representation. JSON null becomes an empty string. Nested arrays and objects are rejected.

## Comparison semantics

The caller selects one unique key column and one or more fields. Keys must be non-empty and unique in both datasets. Matching is exact. Each selected field produces an independent result.

- PASS: a paired key has equal values for the selected field.
- FAIL: a paired key has unequal values.
- INDETERMINATE: a key exists on only one side; the engine does not infer why.

Both key indexes are sorted once per field comparison. A two-pointer merge join emits findings in key order, independent of source row ordering. Complexity is (O(n log n + m log m)) for index construction and (O(n+m)) for the merge pass. Auxiliary index memory is (O(n+m)), excluding parsed tables and findings. The CLI currently rebuilds the indexes for each selected field.

## Evidence and trust boundary

A report records observations made from supplied files under explicit comparison rules. It does not establish that source exports were complete, authentic, correctly extracted or semantically equivalent. The evidence helper binds the two input files and JSON result using SHA-256 hashes and byte sizes. The manifest is unsigned; it detects changes relative to its recorded values but does not establish author identity or protect against replacement of the manifest and files together.

## Website and browser demo

Cloudflare Pages can serve `website/` without a build step. The browser demo accepts CSV only, parses it in the current browser tab, performs the documented exact key/value comparison and offers a JSON download. It does not upload or persist file contents. It is an illustrative browser implementation, not a WebAssembly build of the C library.

## Engineering gates

1. Add property-based and fuzz testing for CSV, JSONL and report parsing.
2. Add sanitizer coverage and representative memory-limit tests.
3. Benchmark large representative exports before publishing throughput guidance.
4. Consider a shared multi-field index to avoid repeating key-index construction.
5. Consider signed manifests only if buyer requirements justify key management and signature verification.
6. Complete independent security and data-handling review before production-assurance claims.
7. Finalise commercial licensing, customer terms and payment operations before accepting customer data or claiming production readiness.
