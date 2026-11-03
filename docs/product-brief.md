# Verity — product and evaluation brief

## Product

Verity is a local-first migration export verification utility for ERP/CRM implementation teams. It compares agreed source and target exports by a unique key, checks selected fields using exact string equality, and produces row-linked findings for human review.

## Buyer and trigger

- **Buyer:** migration delivery director, migration lead or QA lead at an implementation consultancy.
- **Trigger:** a live migration is approaching internal or client sign-off.
- **Current workaround to investigate:** spreadsheets, ad hoc SQL, custom scripts or manual spot checks.
- **Purchase reason to validate:** the team needs a repeatable export comparison and a review artifact without introducing a hosted data platform.

## What is included in a paid evaluation

One fixed-scope evaluation includes:
- one source export and one target export;
- one agreed unique key;
- up to three critical fields;
- CSV or flat-object JSONL;
- comparison JSON;
- self-contained HTML review report;
- SHA-256 manifest for the inputs and generated report;
- exception register and row references;
- a short findings note that separates observed differences from unverified causes.

## What is not included

No database connectors, automated schema mapping, transformation-rule testing, fuzzy matching, numeric tolerances, date normalization, relationship validation, full migration certification, security certification or production deployment.

## Delivery sequence

1. **Qualification:** confirm active migration, file formats, key, fields, buyer, and budget owner.
2. **Safe sample:** buyer supplies synthetic or de-identified sample exports.
3. **Scope confirmation:** both sides agree the comparison rules, outputs, fee, delivery date and data handling in writing.
4. **Paid evaluation:** payment and authorization are confirmed before scoped delivery begins.
5. **Run and review:** produce outputs, inspect exceptions with the buyer, and record unresolved explanations.
6. **Decision:** buyer decides whether to use Verity on another project or stop.

## Acceptance

The evaluation is accepted when:
- the agreed inputs are parsed, or input limitations are documented;
- the unique key is valid and unique in both exports;
- every agreed field receives a result;
- findings include source-row references where available;
- the output is reproducible from the recorded command and tool version;
- the buyer receives the agreed JSON, HTML and evidence files;
- unresolved differences are labelled for human review.

Acceptance means the scoped deliverables were produced. It does not mean the migration is correct or that every finding is a defect.

## Commercial status

The paid evaluation is a proposed service offer, not a proven market price or established customer engagement. Fee, delivery date, payment method, support and any license grant must be agreed before work starts. Verity currently has no published commercial license.

## Go / no-go rule

Proceed beyond the first evaluation only if a buyer with budget confirms that the workflow addresses a recurring sign-off task and is willing to pay for a second project or team use. Do not expand formats or integrations based on hypothetical demand.
