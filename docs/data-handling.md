# Migraven — data handling and evaluation boundaries

This document describes the current product's technical boundaries. It is not a security certification, privacy notice, data-processing agreement or legal contract.

## Processing model

- The comparison executable reads local CSV or flat-object JSONL files.
- The HTML renderer and evidence-manifest helper are local Python scripts using the Python standard library.
- The documented workflow does not require a hosted Migraven account or a Migraven-operated data service.
- The comparison output can contain source values, target values, record keys and row references. Treat the output as sensitive if the input is sensitive.
- The SHA-256 manifest records file paths, hashes and sizes. Paths are relative when possible; on Windows, files on different drive letters require an absolute local path, which is not portable to another machine. The manifest does not encrypt data or establish who created it.

## First evaluation rule

Use synthetic or appropriately de-identified exports for initial evaluation. Do not send production credentials, API keys, database dumps, regulated data or confidential customer records through an informal channel.

Before any real-data evaluation, the buyer and delivery party must agree in writing on:
1. the buyer's authority to share the selected files;
2. permitted data categories and excluded fields;
3. transfer method and access restrictions;
4. named people allowed to access the files;
5. storage location and retention period;
6. deletion and confirmation process;
7. incident contact and escalation path;
8. whether any subcontractor or external service is involved.

If these points cannot be agreed, use synthetic data only.

## Current security limitations

- No independent security assessment or certification.
- No encryption-at-rest feature inside Migraven.
- No role-based access control or multi-user workspace.
- No built-in secure file-transfer service.
- No signed evidence manifests.
- No automatic deletion or retention enforcement.
- No formal audit log of operator actions.
- No guarantee that a buyer's workstation or transfer channel is secure.

The buyer remains responsible for its own authorization, endpoint security, backup, access control and regulatory obligations. Do not represent Migraven as certified, compliant or production-approved.

## Output handling

Comparison JSON and HTML reports may expose values from both datasets. Store them with the same controls as the source exports. The evidence manifest is an integrity aid, not a substitute for secure storage or access control.
