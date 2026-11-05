# Migraven engine contract

## Purpose

The engine compares two agreed exports after a migration or test run. It creates observations for human review. It does not migrate data or certify the migration.

## Inputs

- CSV, or flat-object JSONL.
- One unique key column present in both datasets.
- One to 32 selected comparison fields.
- Non-empty, unique key values on both sides.

CSV and JSONL parsers enforce format-specific structural constraints. Nested JSON arrays and objects are outside the current contract.

## Matching and equality

Keys are matched using exact string equality. Each selected field is compared independently using exact string equality. No trimming, case folding, numeric coercion, date parsing, timezone conversion, currency conversion, fuzzy matching or business-rule inference is performed.

Consequences are intentional:
- `10` and `10.0` differ.
- `Active` and `ACTIVE` differ.
- An empty value and a missing key are not interchangeable.
- A one-sided key is an indeterminate observation, not a confirmed defect.

## Algorithm

1. Resolve the key and selected field column indexes.
2. Build key entries containing a key pointer and original row index.
3. Reject empty or duplicate keys.
4. Sort both key indexes lexicographically.
5. Merge the sorted indexes with two pointers.
6. Emit a field mismatch for paired keys with unequal values.
7. Emit an indeterminate finding for keys present on one side only.

The sorted indexes make the join independent of input row order. Index construction is (O(n log n + m log m)), followed by an (O(n+m)) merge. Auxiliary index memory is (O(n+m)), excluding input tables and emitted findings.

## Output semantics

| Status | Meaning |
|---|---|
| `PASS` | Selected values matched for paired keys; no findings for that field. |
| `FAIL` | A paired key has unequal selected values. |
| `INDETERMINATE` | The key exists in only one input. |

Top-level CLI status is `PASS` only when every selected field has no findings; otherwise it is `REVIEW`. In strict mode, the CLI returns exit code 1 when findings exist. Input or execution errors return exit code 2.

## Evidence

Findings retain original physical source-row references. The separate Python evidence utility hashes the source, target and generated JSON report. The manifest is unsigned and is not proof of authorship, authenticity, completeness or certification.

## Non-goals

- Data movement, transformation or ETL.
- Schema mapping and relationship validation.
- Aggregate reconciliation or business-rule validation.
- Automated root-cause attribution.
- Full migration certification or regulatory sign-off.
