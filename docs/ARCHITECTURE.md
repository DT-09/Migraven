# Architecture

## Current implementation

`verity_core` is a dependency-free C17 static library. The CLI consumes the public header in `include/verity/verity.h`.

- `csv.c`: bounded file loading, CSV tokenisation, table ownership and cleanup.
- `compare.c`: column resolution, duplicate-key detection, exact comparison and finding generation.
- `util.c`: status labels and JSON string escaping.
- `main.c`: command-line parsing and JSON serialization.

## Comparison semantics

The caller selects one key column and one value column. Keys must be non-empty and unique in each dataset. Matched records use exact byte-string equality. Unmatched records produce INDETERMINATE findings because the two datasets alone do not establish why a record is absent.

## Planned extensions

1. Canonical typed record model.
2. Versioned rule registry.
3. JSON and JSONL adapters.
4. Evidence manifests with cryptographic digest and rule/input metadata.
5. Python bindings and stable shared-library ABI.

A digest detects changes relative to a trusted recorded digest; it does not prove source data was truthful when collected.
