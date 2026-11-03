# Verity — Migration Export Verification Evaluation

## Check what changed between source and target exports before sign-off.

Verity compares agreed CSV or flat-object JSONL exports by a unique record key. It checks selected fields, identifies mismatches and missing-side records, and produces a reviewable evidence pack with source-row references.

### Designed for

ERP and CRM implementation consultancies, migration delivery leads and data QA teams working toward a client or internal migration sign-off.

### What the evaluation covers

- One source export and one target export.
- One agreed unique key.
- Up to three agreed critical fields.
- CSV or flat-object JSONL.
- Local comparison using exact string equality.
- Machine-readable JSON results.
- Self-contained HTML review report.
- Exception register with source-row references.
- SHA-256 manifest for the source files and JSON report.
- A concise review note listing observed differences and unresolved questions.

### What the buyer gets

A bounded, reproducible review package—not a vague assessment deck. The buyer can inspect the field-level findings, trace them to source rows, and decide which differences require investigation.

### What Verity does not claim

Verity does not certify a migration, prove business correctness, test transformation logic, normalize dates or numbers, map schemas, connect directly to production systems, or replace user acceptance testing. A finding is not automatically a defect. A PASS applies only to the configured key and fields.

### Evaluation process

1. Confirm the active migration, buyer, budget owner and sign-off requirement.
2. Agree a synthetic or de-identified sample and the key/fields to check.
3. Confirm scope, fee, delivery date, data handling and payment in writing.
4. Run the evaluation and deliver the evidence pack.
5. Review exceptions and decide whether a second project warrants repeat use.

### Data handling

The initial evaluation uses synthetic or appropriately de-identified exports. No production credentials or regulated data are requested. Real-data work requires written authorization and agreed transfer, access, retention and deletion controls. See [Data handling](data-handling.md).

### Commercial next step

Request a scoped evaluation discussion. The fee is quoted after the dataset shape and acceptance requirements are understood. No public price is represented as validated.
