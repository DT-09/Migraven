# Command-line interface

## Inspect

`verity inspect path/to/file.csv`

Returns the input path, header names and data-row count.

## Compare

`verity compare --key record_id --field amount left.csv right.csv`

The key must be non-empty and unique in both files. Each selected field is compared using exact string equality.

Repeat `--field` to compare several columns in one invocation:

```bash
verity compare --key customer_id \
  --field account_status \
  --field region \
  --field credit_limit \
  source.csv target.csv
```

A maximum of 32 distinct fields is accepted. Duplicate `--field` arguments are rejected. Each field receives an independent result with its own summary and findings. The top-level status is `PASS` only if every field has zero findings; otherwise it is `REVIEW`.

### Finding statuses

- `FAIL`: a key exists in both datasets but the selected values differ.
- `INDETERMINATE`: a key exists in only one dataset. This does not establish whether the record is missing, delayed, filtered or legitimately absent.
- `PASS`: the specified value matched for a paired key. A report-level PASS means no findings were generated for any selected field.

### Exit codes

- `0`: command completed, including reports that contain findings.
- `2`: invalid arguments, unreadable input, malformed CSV or failed comparison.

A report with findings is still a successful execution and exits with code 0. Consumers must inspect the JSON status and per-field summaries rather than relying only on the process exit code.
