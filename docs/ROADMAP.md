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

- Package-manager recipes: add recipes after release archives are validated.
  - check: consumer project builds against the published recipe and the release
    archive checksum matches `SHA256SUMS.txt`
- Run focused external security review after the release trust pass.
  - check: findings map to an existing API, CLI command, serialized format, CMake
    package surface, release asset, or security-reporting surface
- Password-file KDF profile design: write the spec before adding another profile.
  - check: spec covers format bytes, downgrade behavior, bounds, fixture policy,
    and at least three known-answer vectors
- Additional streaming format threat model: decide plaintext-before-auth and output
  ownership before adding any format.
  - check: threat model maps to `docs/FORMAT.md`, `docs/SECURITY_MODEL.md`, and
    negative fixtures
- Scheduled fuzzing plan: promote only after `fuzz-smoke` gives repeated useful
  signal and an owner exists.
  - check: scheduled job has owner, runtime limit, artifact policy, and failure
    triage path
