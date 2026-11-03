# Verity — paid evaluation proposal

> Complete every bracketed field and obtain appropriate review before sending. This is a proposal template, not a legal contract.

## 1. Parties and project

- Buyer organization: [organization]
- Buyer sponsor / budget owner: [name and role]
- Delivery contact: [name and role]
- Provider: [legal contracting party]
- Migration project: [project / source and target systems]
- Proposal date: [date]
- Valid until: [date]

## 2. Objective

Run a bounded comparison of agreed source and target exports to identify selected field-level differences and provide traceable findings for the buyer's migration team to review.

## 3. Included scope

- Source export: [filename, format, row count]
- Target export: [filename, format, row count]
- Unique key: [field]
- Fields checked (maximum three): [field list]
- Comparison rule: exact string equality
- Tool version: [version]
- Delivery date: [mutually agreed date]

## 4. Deliverables

- Input inventory and run configuration.
- Machine-readable comparison JSON.
- Self-contained HTML review report.
- Exception register with source-row references.
- SHA-256 evidence manifest for both inputs and the JSON report.
- Review note distinguishing observed differences from unresolved explanations.

## 5. Acceptance criteria

The evaluation is complete when the agreed files have been processed or limitations documented; the agreed key and fields have results; available findings include row references; the run can be reproduced from the recorded command and tool version; and all listed deliverables are supplied.

Acceptance confirms delivery of the scoped outputs. It does not certify migration correctness or confirm that each finding is a defect.

## 6. Buyer responsibilities

The buyer confirms authority to provide the files, supplies accurate field definitions and comparison rules, identifies an authorized reviewer, and provides synthetic or appropriately de-identified data unless separate written data-handling terms are agreed.

## 7. Data handling

- Permitted data: [describe]
- Excluded data: [describe]
- Transfer method: [method]
- Authorized access: [people]
- Storage location: [location]
- Retention period: [period]
- Deletion method and confirmation: [method]
- Incident contact: [contact]

Do not begin processing until authorization and data-handling arrangements are agreed.

## 8. Exclusions

Database access, production credentials, integrations, schema mapping, transformation-rule testing, fuzzy matching, numeric/date normalization, relationship checks, full migration certification, compliance certification and security assessment are excluded unless separately scoped in writing.

## 9. Commercial terms

- Evaluation fee: [amount and currency]
- Taxes / fees: [treatment]
- Payment milestone: [e.g. advance before work begins]
- Payment method: [agreed method]
- Delivery schedule: [schedule]
- Change requests: [process]

No work begins until scope, authorization and payment arrangements are confirmed.

## 10. Approval

Buyer representative: [name / role / date]

Provider representative: [name / role / date]
