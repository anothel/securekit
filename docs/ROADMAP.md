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
runnable check, and a rollback path. Put accepted fixes in `Fix Queue` until
they move to `Now`. Do not leave accepted fixes deferred.

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
  recorded, do not promote a deferred split or API expansion.

### Fix Queue

After the `Now` release-confidence pass, work through these fixes. Each item
must keep the current public API and format contracts unless its own check proves
the change is required.

- `src/file.cpp` internal split: split internals behind the same public file APIs:
  - format parse/serialize
  - password KDF/header handling
  - chunk AEAD
  - path/temp-file commit
  - verify-only paths
  - check: `cmake --build build --config Release --target release-preflight`
- CLI split: move command handling out of `src/cli/main.cpp` without changing command
  shape or exit codes.
  - check: `cmake --build build --config Release --target release-preflight`
- README split into `docs/CLI.md` or `docs/API.md`: move detail out, leaving README as the
  short entry point.
  - check: `cmake --build build --config Release --target release-preflight`
- Package-manager recipes: add recipes after release archives are validated.
  - check: consumer project builds against the published recipe and the release
    archive checksum matches `SHA256SUMS.txt`
- Add benchmarks for crypto/file paths after the release checks stay green.
  - check: benchmark target builds and reports stable local measurements
- Expand negative compatibility fixtures for uncovered `FORMAT.md` reject rules.
  - check: compare `docs/FORMAT.md` with `tests/fixtures/negative/README.md`,
    then run `cmake --build build --config Release --target release-preflight`
- Run focused external security review after the release trust pass.
  - check: findings map to an existing API, CLI command, serialized format, CMake
    package surface, release asset, or security-reporting surface

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

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Web framework middleware, including Express, Koa, Fastify, NestJS, JWT, CSRF,
  CORS, rate limiting, request validation, or diagnostic web routes.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
- Result-style public APIs unless a breaking-change proposal proves exceptions
  are the wrong boundary for current users.
- Object-oriented APIs beyond packet streaming unless current call sites require
  lifecycle state that free functions cannot express.
- New password-file KDF profile until there is a format spec, downgrade behavior,
  bounds, fixture policy, and at least three known-answer vectors.
- Additional streaming format until there is a written threat model for
  plaintext-before-auth and output ownership.
- OpenSSL provider or FIPS helpers until there is a documented support policy.
- Scheduled long-running fuzz until `fuzz-smoke` produces repeated useful signal
  and someone owns the scheduled job.
