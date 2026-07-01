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
8. Keep out-of-scope boundaries as rules, not queued work: no custom crypto
   primitives, custom string classes or allocators, TLS/networking, secure key
   storage, guaranteed memory erasure claims, or framework-scale abstractions
   without call-site pressure.

Release-impacting work must pass the matching configured build directory:

```sh
cmake --build build --config Release --target release-preflight
```

Before tagging a release, verify package archives, source archives,
`SHA256SUMS.txt`, release SPDX SBOM, GitHub artifact attestations, and release
notes source of truth.

`release-preflight` includes `dogfood-check`. If no repeated friction is
recorded, do not promote an API expansion or split.

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

Package Publishing:

- Package-manager recipe publication is the active package-channel track.
- Publish generated Homebrew, Conan, and vcpkg recipes only after the matching
  GitHub Release assets are uploaded and attested.
- Verify each recipe source URL points at the released
  `securekit-X.Y.Z-source.tar.gz` and its checksum matches `SHA256SUMS.txt`.
- check: consumer project builds against each published recipe
- rollback: remove or replace the package recipe update if checksum or consumer
  build verification fails

### Fix Queue

After the `Now` package publication pass, work through these fixes. Each item
must keep the current public API and format contracts unless its own check
proves the change is required.

- Run focused external security review after the package publication pass.
  - surfaces: `SKT1` packet parser/reject rules, `SKF1`/`SKP1` file parsing,
    path output safety, stream/stdout output ownership, AAD handling,
    password-derived key handling, and release/security-reporting docs
  - check: findings map to an existing API, CLI command, serialized format, CMake
    package surface, release asset, or security-reporting surface
  - rollback: keep un-mapped findings as triage notes until they name a
    SecureKit surface and regression check

- Turn accepted external-review findings into one protected change each.
  - surfaces: public C++ APIs, CLI commands, `SKT1`/`SKF1`/`SKP1` fixtures,
    `tests/fuzz/corpus`, and `docs/SECURITY_MODEL.md`
  - check: each fix adds or names a regression test, negative fixture, or
    minimized corpus seed before `release-preflight`
  - rollback: revert the smallest finding fix if its regression check is wrong
    or changes public API/format behavior without an approved gate

- Record a coverage baseline for security-critical branches before adding any
  percentage gate.
  - surfaces: `coverage-report`, `docs/COVERAGE.md`, parser reject paths, file
    rollback paths, CLI error mapping, and OpenSSL backend failure handling
  - check: `coverage-report` output produces a baseline note and a checklist of
    uncovered critical branches
  - rollback: keep coverage observational if the baseline is unstable or lacks
    an owner

- Revisit file implementation boundaries only when review or repeated edits
  show the current split is still too hard to maintain.
  - surfaces: `src/file.cpp`, `src/file_detail.hpp`, `src/file_crypto.cpp`,
    `src/file_io.cpp`, and `docs/INTERNALS.md`
  - check: `check` and `release-preflight` pass with no public C++ API, CLI, or
    `SKT1`/`SKF1`/`SKP1` format change
  - rollback: stop at the current split if the change only moves code without
    reducing a named review or maintenance risk
