# SecureKit Roadmap

Future work only. Completed work belongs in Git history and release notes, not a
standing changelog.

## Rules

1. Security, data loss, and format compatibility come first.
2. Keep v0.x public API changes minimal and keep the v1 free-function API
   stable unless real call sites prove otherwise.
3. Keep OpenSSL as the crypto backend.
4. Keep CMake install/export, the CLI, and GitHub Actions package checks as
   first-class release surfaces.
5. Do not add public API, wire formats, package channels, CI cost, or release
   ceremony without a written problem, regression check, and rollback plan.
6. Active roadmap items must name an existing SecureKit surface.
7. External audit or roadmap notes are triage input only. Notes for Node.js
   backend middleware repositories, including `package.json`, npm,
   `TEST_SUMMARY`, Express, Koa, Fastify, NestJS, JWT, CSRF, CORS, rate
   limiting, request validation, diagnostic routes, or adapter parity, do not
   become SecureKit work unless this repository's identity changes.

Release-impacting work must pass the matching configured build directory:

```sh
cmake --build build --config Release --target release-preflight
```

## External Analysis Triage

Broad audit documents must be split before implementation. Do the smallest
repo-relevant work first, then leave the rest parked or not planned. The
2026-06-28 SecureKit analysis is tracked here as maintenance and release
confidence work, not as a feature expansion plan.

| Audit theme | SecureKit handling | Required check |
| --- | --- | --- |
| Docs and implementation mismatch | Keep README, `FORMAT.md`, `SECURITY_MODEL.md`, release checklist, release notes, and public headers aligned with the C++ library and CLI. | `release-preflight` |
| Public contract comments | Keep path open, stream rollback, password byte handling, decrypt authentication, HKDF output size, and constant-time length caveats visible from public headers. | `public_headers_test`, `release-preflight` |
| Package and release trust | Before tagging v0.2.0 or later, verify install/export/package archives, release assets, checksums, SBOM, provenance wiring, and release notes source of truth. | `package-check`, `release-workflow-check`, `release-preflight` |
| Test gates and negative cases | Add regression coverage only for an existing SecureKit API, CLI, format, package, or release bug. | Smallest matching unit, CLI, fixture, or package check |
| Internal boundary pressure | Keep `src/file.cpp` and `src/cli/main.cpp` split gates documented before refactoring. | `docs/INTERNALS.md`, `release-preflight` |
| Fuzz and parser hardening | Keep fuzz smoke optional and corpus policy explicit; schedule long runs only with useful signal and an owner. | `fuzz-smoke` when configured |
| New API shape | Park until real call sites prove the need and name one runnable check. | Item-specific gate below |
| Web middleware findings | Not planned for this repository. | Roadmap scope guard |

Unrelated Node.js backend middleware repositories remain out of scope unless
the project identity changes: NestJS exports, Express/Koa/Fastify adapter parity,
CORS, CSRF, JWT, security event routing, diagnostic routes, sanitization,
file-upload validation, distributed rate limiting, public cache defaults, npm
package exports, `package.json`, `TEST_SUMMARY`, and CLI audit/config placeholders.

### 2026-06-28 SecureKit Analysis Disposition

This table records the item-by-item triage result for
`securekit_analysis_2026-06-28.md`. "Accepted" means the theme belongs in this
roadmap; it does not mean immediate implementation.

| Analysis item | Disposition | Reason / gate |
| --- | --- | --- |
| Run `release-preflight` on the current tree | Accepted as a required verification, not standing feature work. | Release-impacting work already requires `release-preflight`; passing runs belong in the final report or release notes, not the roadmap. |
| `CHANGELOG.md` vs `docs/RELEASE_NOTES.md` source of truth | Already resolved. | `docs/RELEASE_NOTES.md` is the source of truth; release checklist says not to add a separate `CHANGELOG.md` without changing the policy. |
| `main` v0.2.0 state vs public release/tag state | Accepted as release-trust work. | Keep only as release execution/verification; local tags are not enough to prove public GitHub Release assets, checksums, SBOM, or provenance. |
| README release archive examples | Accepted under package and release trust. | Covered by `release-preflight` and `release-workflow-check`; change only on a concrete mismatch. |
| Public header contract comments | Already resolved, keep guarded. | Headers now document path/stream output ownership, password byte handling, decrypt authentication, HKDF output size, and constant-time length caveats. |
| README `SECUREKIT_BUILD_FUZZ` visibility | Already resolved, keep guarded. | README links the option to `docs/FUZZING.md`; preflight checks the option and link. |
| `packet_stream` internal key/AAD cleanup | Already resolved, keep guarded. | `packet_encryptor::impl` and `packet_decryptor::impl` wipe stored key and AAD on destruction; `InternalWipe` covers the wipe helper. |
| `FORMAT.md` reject rules vs negative fixtures | Accepted as compatibility hardening, mostly satisfied. | Existing fixture inventory checks documentation and coverage matrix; add fixtures only for a specific uncovered reject rule. |
| CLI help and README synchronization | Accepted as release-confidence work. | Keep through `release-preflight`; add stricter checks only after a concrete CLI/docs drift. |
| `src/file.cpp` internal split | Parked. | Split only after repeated edit pressure or safety work proves the smaller module boundary. |
| `src/cli/main.cpp` split | Parked. | Split only after repeated edit conflicts or churn, not just file size. |
| README split into `docs/CLI.md` or `docs/API.md` | Parked. | Split only after repeated reader or edit friction. |
| Result-style APIs | Parked. | Needs two real call sites proving exceptions are the wrong boundary. |
| Larger object-oriented APIs | Parked. | Needs two real call sites duplicating lifecycle logic that free functions cannot express. |
| Options structs for algorithms, chunk size, or KDF tuning | Not planned for now. | Current fixed choices are part of the security boundary; revisit only with a written threat model and compatibility plan. |
| New password-file KDF profile | Parked. | Needs format spec, downgrade behavior, bounds, fixture policy, and at least three known-answer vectors. |
| Additional streaming format | Parked. | Needs a written threat model for plaintext-before-auth and output ownership. |
| FIPS/provider helpers | Parked. | Needs documented support policy and dedicated tests. |
| Scheduled long-running fuzz | Parked. | Needs repeated useful `fuzz-smoke` signal and an owner. |
| Latest OpenSSL 3.x/provider matrix expansion | Not queued. | Keep OpenSSL 3.x support broad but avoid CI matrix cost without a provider/FIPS policy or concrete compatibility failure. |
| Release workflow strictness cleanup | Not queued. | Current string checks intentionally catch release drift; improve only for a real false positive or unclear failure. |
| External security review or formal audit | Parked. | Needs a v1-facing threat model, stable fixtures, release artifact verification, and a findings owner. |
| Package manager distribution | Parked. | Needs validated release archives and a real consumer request. |
| Benchmarks | Parked. | Needs stable correctness, format, and release checks first. |

### 2026-06-28 Full SecureKit Analysis Disposition

This table records follow-up triage for
`securekit_full_analysis_2026-06-28.md`. "Done" means the current tree now has a
matching artifact or check.

| Analysis item | Disposition | Reason / gate |
| --- | --- | --- |
| README audit and high-risk-use posture | Done. | README now states SecureKit is not externally audited and is not a substitute for a full high-risk security review. |
| Safe default API selection guide | Done. | README now has a `Which API Should I Use?` table that points common users at one-shot and path-based APIs before streaming APIs. |
| Streaming/plaintext-before-auth warning prominence | Done. | README now leads packet streaming with a safe-default note; existing packet and file warnings remain in README and `docs/SECURITY_MODEL.md`. |
| User-facing release verification guide | Done. | `docs/VERIFY_RELEASE.md` documents checksum, attestation, SBOM, and source-vs-binary verification. |
| `FORMAT.md` reject rules vs negative fixture matrix | Already resolved. | `tests/fixtures/negative/README.md` has the coverage matrix and release preflight checks it. |
| Maintenance/Reliability backlog visibility | Done. | See `Maintenance State`; completed reliability work is separated from parked split candidates. |
| Format-check target or CI | Done. | `format-check` runs `clang-format --dry-run --Werror` when clang-format is available. |
| Known-answer vector provenance | Done. | `tests/fixtures/README.md` now records provenance and regeneration rules without changing fixture bytes. |
| Coverage baseline | Done. | `coverage-report` is available as a non-blocking GCC/Clang + gcovr local report target. |
| Dependency/update hygiene | Done. | `.github/dependabot.yml` covers GitHub Actions and `docs/DEPENDENCY_POLICY.md` covers Actions, GoogleTest, and OpenSSL update policy. |
| README split | Accepted only if reader friction is real. | Prefer moving release verification to docs first; split CLI/API only when README remains hard to scan. |
| `src/file.cpp` internal split | Accepted behind gate. | Split only with byte-for-byte fixture regression checks and a narrow internal boundary. |
| CLI helper split | Accepted behind gate. | Extract command helpers only when it reduces real command/error mapping churn. |
| Result-style API, broader OO API, new KDF profile, new streaming format, FIPS helpers, package channels, benchmarks | Not planned for now. | These expand public or operational surface; revisit only with the gates already listed in Parked. |

## Maintenance State

These are reliability tasks, not feature expansion.

| Item | State | Check |
| --- | --- | --- |
| Add a small `format-check` target or CI job. | Done as local target. | `cmake --build build --config Release --target format-check` |
| Document known-answer vector provenance and fixture regeneration notes. | Done. | `FixtureInventory.*`, `release-preflight` |
| Add dependency/update hygiene policy. | Done. | `release-workflow-check`, `release-preflight` |
| Add non-blocking coverage reporting. | Done as local report target. | `cmake --build build-coverage --target coverage-report` |
| Prepare `src/file.cpp` internal split. | Parked until repeated edit pressure or safety work proves the split. | `release-preflight` |
| Prepare CLI command/helper split. | Parked until repeated command/error mapping churn is recorded. | CLI package checks |

## Now

No active queued work. `dogfood-check` covers the v0.2.0 post-release
consumer loop; no repeated friction is recorded. A parked item has a proven gate
only after real repeated friction is recorded, and then can move to `Next`.

## Next

No queued feature work. After the `Now` release-confidence pass, promote at
most one parked item only after dogfooding records real repeated friction,
proves the item's gate, and names one runnable check.

## Parked

These are blocked candidates, not queued work. Move one to `Now` or `Next` only
after its gate is proven and a runnable check is named.

- Result-style APIs: gate is two real call sites showing exceptions are the
  wrong boundary.
- Object-oriented APIs beyond packet streaming: gate is two real call sites
  duplicating lifecycle logic that free functions cannot express cleanly.
- New password-file KDF profile: gate is format spec, downgrade behavior,
  bounds, fixture policy, and at least three known-answer vectors.
- Additional streaming format: gate is a written threat model for
  plaintext-before-auth and output ownership.
- OpenSSL provider or FIPS helpers: gate is documented support policy and
  dedicated tests.
- Scheduled long-running fuzz: gate is repeated useful `fuzz-smoke` signal and
  an owner for the scheduled job.
- Further negative compatibility fixture expansion: gate is a specific
  uncovered `FORMAT.md` reject rule found by comparing `docs/FORMAT.md` with
  `tests/fixtures/negative/README.md`.
- External security review or focused audit: gate is a v1-facing threat model,
  stable compatibility fixtures, release artifact verification, and an owner
  for findings triage.
- `src/file.cpp` internal split: gate is repeated local edit pressure or safety
  work that is simpler after separating format parse/serialize, KDF, chunk
  AEAD, temp-file commit, and password header handling.
- CLI split: gate is repeated edit conflicts in `src/cli/main.cpp`.
- README split into `docs/CLI.md` or `docs/API.md`: gate is repeated reader or
  edit friction, not file length alone.
- Package-manager recipes: gate is validated release archives plus a real
  consumer request for a package-channel recipe.
- Benchmarks: gate is stable correctness, format, and release checks.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Web framework middleware, including Express, Koa, Fastify, NestJS, JWT, CSRF,
  CORS, rate limiting, request validation, or diagnostic web routes.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
