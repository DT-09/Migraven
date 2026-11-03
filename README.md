# VERITY

**Infrastructure for verifiable data.**

VERITY is a local-first C17 data verification engine. The initial MVP provides CSV inspection and deterministic cross-source comparison, with machine-readable findings and source-row references.

> Status: early development / MVP. Not production-certified. No performance, security-certification, or customer claims are made.

## Capabilities

- C17 static library and CLI.
- CSV parsing with quoted fields, escaped quotes and CRLF.
- Header and row-width validation.
- Cross-source comparison using a selected unique key and value column.
- Duplicate-key rejection.
- PASS, FAIL and INDETERMINATE findings.
- JSON output with source row references.
- CMake build and automated tests.

## Build

Requirements: CMake 3.16+ and a C17 compiler.

```bash
cmake -S . -B build -DVERITY_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Use

```bash
verity inspect examples/left.csv
verity compare --key id --field amount examples/left.csv examples/right.csv
```

Comparison currently uses exact string equality. It does not perform currency conversion, numeric tolerance, fuzzy matching or business-rule inference. Missing keys are INDETERMINATE; unequal matched values are FAIL.

## Design boundaries

- Local-first execution.
- Stable input-order results.
- Source-row traceability.
- Conservative handling of malformed input and duplicate keys.
- A passing rule proves only that the specified comparison passed.

## Roadmap

1. Harden parser and comparison contracts.
2. Add JSON/JSONL ingestion and typed canonical records.
3. Add versioned rules and numeric comparisons.
4. Add cryptographic input fingerprints and reproducible evidence manifests.
5. Add Python bindings and stable shared-library ABI.
6. Add fuzzing, sanitizers, CI and reproducible benchmarks.

## License

No license has been selected. Normal copyright restrictions apply; do not assume open-source or commercial reuse rights.