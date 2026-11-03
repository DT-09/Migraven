# Verity — first market

## Beachhead

**Independent migration verification for ERP/CRM implementation consultancies.**

Initial buyer: a small or mid-sized implementation consultancy that moves customer or operational records between systems and must demonstrate that selected critical fields survived the transfer.

Economic buyer: delivery director, migration lead, or QA lead.

Daily user: migration engineer or data QA analyst.

## The job

Before a migration is signed off, compare an agreed source export with the target export and produce a reviewable exception list.

Verity's first supported job is deliberately narrow:
- Read two CSV exports locally.
- Match records by a unique, non-empty key.
- Compare one agreed critical field per run using exact string equality.
- Identify unequal values and keys present on only one side.
- Preserve source row references in machine-readable output.
- Leave interpretation and acceptance with the migration team.

Run one check per critical field. The operator retains the separate JSON outputs as the evidence bundle.

## Why this wedge

Large migration-testing platforms already sell broad connectors, workflow automation, transformation testing, dashboards and enterprise reporting. Verity should not pretend to replace them.

The initial wedge is a lightweight, local-first verification utility for a consultancy that needs a transparent, repeatable check on CSV exports without introducing another hosted data platform.

## Buyer qualification

A prospect is qualified only when all are true:
1. They deliver ERP, CRM, finance-system or data-warehouse migrations.
2. They can provide a non-sensitive, representative source/target CSV pair or synthetic equivalent.
3. They have an agreed unique record key and named critical fields.
4. They need an independent comparison artifact for internal or client review.
5. A responsible buyer can approve a paid, fixed-scope evaluation.

Do not target companies merely because they use data. Target migration delivery teams with an active project and a sign-off requirement.

## Positioning

**Verity produces local, source-row-linked checks between agreed CSV migration exports.**

It is not currently:
- a database connector or ETL platform;
- a full schema-mapping or transformation-rule engine;
- a cryptographic proof or tamper-evident audit system;
- a regulated validation platform;
- a guarantee that a migration is correct.

## Competitive boundary

Products such as QuerySurge and iceDQ describe broad data-migration testing capabilities, including multiple data sources, reusable rules, workflows and reporting. Verity's current MVP is materially narrower. Compete on a small, inspectable CSV verification task and a transparent delivery process—not feature count.

## Expansion gate

Do not add integrations, dashboards or more formats until a paid pilot demonstrates:
- a repeatable dataset shape;
- fields buyers actually need checked;
- an acceptable evidence format;
- time saved versus the buyer's current manual process;
- a second project or referral opportunity.
