# Command-line interface

## Inspect

CSV is the default:

    migraven inspect source.csv

JSONL must be selected explicitly:

    migraven inspect --format jsonl source.jsonl

The command returns the input path, format, column names and data-row count.

## Compare

    migraven compare --key record_id --field amount source.csv target.csv\n\nUse `--strict` when findings should return exit code 1:\n\n    migraven compare --strict --key record_id --field amount source.csv target.csv\n\nUse `--strict` when the process should return a non-zero status if findings exist:\n\n    migraven compare --strict --key record_id --field amount source.csv target.csv

The key must be non-empty and unique in both inputs. Each selected field is compared using exact string equality.

For JSONL:

    migraven compare --format jsonl --key record_id --field amount source.jsonl target.jsonl

JSONL input is a sequence of flat JSON objects. Every object must contain the same property names; property order may differ. Supported values are strings, numbers, booleans and null. Nested arrays and objects are rejected.

Repeat --field to compare several columns in one invocation:

    migraven compare --key customer_id \
      --field account_status \
      --field region \
      --field credit_limit \
      source.csv target.csv

A maximum of 32 distinct fields is accepted. Duplicate --field arguments are rejected. Each field receives an independent result with its own summary and findings. The top-level status is PASS only if every field has zero findings; otherwise it is REVIEW.

## Finding statuses

- FAIL: a key exists in both datasets but the selected values differ.
- INDETERMINATE: a key exists in only one dataset. This does not establish whether the record is missing, delayed, filtered or legitimately absent.
- PASS: the specified value matched for a paired key. A report-level PASS means no findings were generated for any selected field.

## Exit codes

- 0: command completed, including reports that contain findings.
- 2: invalid arguments, unreadable input, malformed input or failed comparison.

A report with findings is still a successful execution and exits with code 0. Consumers must inspect the JSON status and per-field summaries rather than relying only on the process exit code.


## Result provenance

Comparison JSON includes the source and target filenames and parsed row counts. It deliberately records filenames rather than full local paths, so reports do not expose workstation directory structures. Findings retain source and target row references for review.
