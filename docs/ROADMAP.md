# SecureKit Roadmap

Future work only. Completed work belongs in Git history and
`docs/RELEASE_NOTES.md`, not in a standing changelog.

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
7. External audit or roadmap notes are triage input only. Node.js backend middleware,
   npm/package metadata, web framework middleware, JWT, CSRF, CORS, rate limiting,
   request validation, diagnostic routes, and adapter parity do not become
   SecureKit work unless this repository's identity changes.

Release-impacting work must pass the matching configured build directory:

```sh
cmake --build build --config Release --target release-preflight
```

## Intake Rules

Broad audit documents must be split before implementation. Each accepted item
must map to an existing SecureKit API, CLI command, serialized format, CMake
package surface, release asset, or security-reporting surface.

Put work in `Now` only when it has a named surface, a concrete problem, one
runnable check, and a rollback path. Put work in `Next` only when the `Now`
gate is done and the next item has evidence. Put speculative work in `Parked`.
A parked item has a proven gate only after real repeated friction is recorded
and one runnable check is named.

Completed items leave the roadmap. Use Git history and `docs/RELEASE_NOTES.md` for full completed-change detail.

## Current Plan

### Now

Release Trust:

- Package and release trust is the active release-confidence track.
- Run `release-preflight` on the current tree before release-impacting work is
  called complete.
- Before tagging v0.2.0 or later, verify package archives, source archives,
  `SHA256SUMS.txt`, release SPDX SBOM, GitHub artifact attestations, and release
  notes source of truth.
- Run `dogfood-check` after package generation. If no repeated friction is
  recorded, do not promote a parked split or API expansion.

### Next

No queued feature work. After the `Now` release-confidence pass, promote at
most one parked item only after dogfooding records real repeated friction,
proves the item's gate, and names one runnable check.

## Recently Finished

This is a short orientation list, not a changelog. Use Git history and `docs/RELEASE_NOTES.md` for full completed-change detail.

- Public README, `FORMAT.md`, `SECURITY_MODEL.md`, release checklist, release
  notes, and public headers were aligned with current C++ APIs, CLI commands,
  and `SKT1`/`SKF1`/`SKP1` formats.
- `verify-file` and `verify-file-password` CLI paths were added to verify
  encrypted files without creating plaintext output files.
- Negative compatibility fixture inventory, known-answer vector provenance,
  dependency/update hygiene, local `format-check`, non-blocking coverage report,
  release asset/SBOM checks, and `dogfood-check` are now covered by local
  release checks.
- `docs/INTERNALS.md` records the split gates for `src/file.cpp` and
  `src/cli/main.cpp`; no split is queued without repeated friction.

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
