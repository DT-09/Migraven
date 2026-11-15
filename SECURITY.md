# Security policy

## Scope

Migraven is evaluation-stage software. It is not independently security-assessed or certified. The current product has no hosted customer-data service, account system, role-based access control, encryption-at-rest feature, signed evidence manifest or built-in secure file-transfer service.

## Reporting a vulnerability

Please do not publish exploitable details in a public issue.

Use GitHub's private vulnerability reporting for this repository if it is enabled. If private reporting is unavailable, contact the maintainer through the GitHub profile and ask for a private reporting route. Include the affected version or commit, impact, reproduction conditions and a minimal proof of concept. Do not attach customer data, credentials or secrets.

## Handling guidance

- Test with synthetic or properly de-identified data.
- Do not submit production credentials, database dumps or confidential exports.
- Treat comparison JSON, HTML reports and manifests as sensitive when their source inputs are sensitive.
- A SHA-256 manifest is an integrity aid, not proof of authorship or authenticity.

## Response expectations

The maintainer will assess reports as capacity allows. No response-time SLA or coordinated-disclosure deadline is currently promised. Please allow a reasonable period for triage before public disclosure.
