# Migraven release readiness checklist

This checklist separates a technically reproducible release from a commercially or security-approved deployment. Passing CI does not establish the latter.

## 1. Source and build integrity

- [ ] Release tag points to the reviewed commit.
- [ ] GCC and Clang CI jobs pass.
- [ ] AddressSanitizer and UndefinedBehaviorSanitizer job passes.
- [ ] Cross-platform build and test matrix passes.
- [ ] Website navigation, local links, assets and JavaScript syntax checks pass.
- [ ] Release bundles contain the executable, README, documentation, examples and local demonstration utilities.
- [ ] SHA-256 checksum files are generated and independently checked after download.
- [ ] Release notes identify supported platforms, known limitations and exact version.

## 2. Functional acceptance

- [ ] CSV parsing and flat-object JSONL parsing pass the regression suite.
- [ ] Duplicate and empty keys are rejected.
- [ ] Missing columns and malformed records are rejected.
- [ ] Exact-string semantics are documented and tested.
- [ ] Source-row references remain stable and traceable.
- [ ] JSON output parses and report generation escapes untrusted values.
- [ ] Evidence verification detects missing or modified inputs and reports that authenticity is not established.
- [ ] The synthetic end-to-end demonstration is reproducible from the release bundle.

## 3. Product and website accuracy

- [ ] Browser demo is clearly labelled as a browser-side demonstration.
- [ ] The website does not imply that browser-selected files are uploaded.
- [ ] The website distinguishes observations from confirmed defects.
- [ ] PASS is scoped to selected keys and fields, not overall migration correctness.
- [ ] No customer logos, testimonials, performance claims or savings claims appear without permission and evidence.
- [ ] Enquiry behavior is described accurately: it prepares a local email draft and does not submit a lead to a server.

## 4. Security, privacy and legal gates

These are not satisfied by a successful software build.

- [ ] Independent security review completed for the intended deployment context.
- [ ] Data-transfer, access, storage, retention and deletion controls agreed before any real customer data is handled.
- [ ] Customer authorization and data-processing terms reviewed and signed where required.
- [ ] Commercial licence and redistribution rights finalized in writing.
- [ ] Support scope, update policy and incident contact defined.
- [ ] Any required privacy notice and contractual terms published.
- [ ] No claim of certification or regulatory compliance is made without a valid basis.

## 5. Commercial readiness

- [ ] A specific buyer, active migration checkpoint and budget owner are identified.
- [ ] Evaluation scope, acceptance criteria, fee, payment route and delivery date are agreed in writing.
- [ ] The buyer approves the use of synthetic or properly de-identified data for initial evaluation.
- [ ] Deliverables and reviewer responsibilities are explicit.
- [ ] A repeat-use case is demonstrated before proposing recurring licensing.

## Current boundary

The repository provides a working evaluation-stage toolchain and a static website. It does not currently include payment processing, subscriptions, hosted lead capture, a published commercial licence, a customer data-processing agreement or independent security certification. Do not describe the product as enterprise-approved or production-certified until the relevant gates have actually been completed.
