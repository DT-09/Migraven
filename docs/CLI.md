# Command-line interface

## Inspect

`verity inspect path/to/file.csv`

Returns the input path, format, header names and data-row count. CSV is the default. Use `verity inspect --format jsonl path/to/file.jsonl` for JSON Lines.

## Compare

`verity compare --key record_id --field amount left.csv right.csv`

The key must be non-empty and unique in both files. Each selected field is compared using exact string equality.

For JSONL, pass `--format jsonl`:

```bash
verity compare --format jsonl --key customer_id \
  --field account_status \
  --field region   source.jsonl target.jsonl
```

JSONL input consists of one object per non-empty line. Each object must contain the same property names in the same order. String, number, boolean and null scalar values are supported. Nested objects, arrays and schema changes are rejected. CSV remains the default input format.

Repeat `--field` to compare several columns in one invocation:

```bash
verity compare --key customer_id \
  --field account_status \
  --field region \
  --field credit_limit \
  source.csv target.csv
```

A maximum of 32 distinct fields is accepted. Duplicate `--field` arguments are rejected. Each field receives an independent result with its own summary and findings. The top-level status is `PASS` only if every field has zero findings.

### Finding statuses

- `FAIL`: a key exists in both datasets but the selected values differ.
- `INDETERMINATE`: a key exists in only one dataset. This does not establish whether the record is missing, delayed, filtered or legitimately absent.
- `PASS`: the specified value matched for a paired key. A report-level PASS means no findings were generated for any selected field.

### Exit codes

- `0`: command completed, including reports that contain findings.
- `2`: invalid arguments, unreadable input, malformed input or failed comparison.

A report with findings is still a successful execution and exits with code 0. Consumers must inspect the JSON status and per-field summaries rather than relying only on the process exit code.
