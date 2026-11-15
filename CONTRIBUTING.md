# Contributing to Migraven

Migraven is a small, auditable C17 comparison engine with Python evidence tooling and a static browser demonstration. Changes should preserve deterministic behavior and keep claims aligned with implementation.

## Development setup

Requirements: CMake 3.16+, a C17 compiler (GCC, Clang or MSVC), and Python 3.10+.

~~~bash
cmake -S . -B build -DMIGRAVEN_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --config Release --output-on-failure
python tools/demo.py
python tests/test_website.py
node --check website/assets/site.js
~~~

## Engineering rules

1. Keep the native engine dependency-free and C17-compliant.
2. Preserve exact-string comparison unless a separately documented contract change is approved.
3. Never silently normalize keys, values, dates, currencies or whitespace.
4. Preserve source-row references and deterministic finding order.
5. Reject malformed input explicitly; do not silently repair ambiguous records.
6. Keep browser-demo claims separate from production C17 capabilities.
7. Do not add network transmission or persistence to the browser demo without an explicit product and privacy review.
8. Do not add customer, performance, security or compliance claims without reproducible evidence.

## Tests

Every behavior change should include regression coverage. Native changes belong in the C tests or CLI tests. Python utilities need focused tests. Website changes must pass local-link, asset, navigation and JavaScript syntax checks. Run the full CTest suite before opening a pull request.

## Pull requests

Include the problem and expected behavior, implementation boundary, tests added or run with actual results, compatibility implications, and documentation updates for contract changes.

Do not include real customer exports, secrets, access tokens or confidential records in commits or test fixtures.
