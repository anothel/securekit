# SecureKit Roadmap

Future work only. Completed work belongs in Git history and release notes, not a
standing changelog.

## Rules

1. Security, data loss, and format compatibility come first.
2. Keep the v1 free-function API stable unless real call sites prove otherwise.
3. Keep OpenSSL as the crypto backend.
4. Keep CMake install/export, the CLI, and GitHub Actions package checks as
   first-class release surfaces.
5. Do not add public API, wire formats, package channels, CI cost, or release
   ceremony without a written problem, regression check, and rollback plan.

Release-impacting work must pass the matching configured build directory:

```sh
cmake --build build --config Release --target release-preflight
```

## Now

### 1. Keep Claims, Contracts, and Tests Aligned

Goal: keep the public story no broader than the verified C++ library surface.

- Keep README feature claims, `docs/SECURITY_MODEL.md`, `docs/FORMAT.md`, and
  release packaging checks aligned with real code.
- Keep API, CLI, CMake install/export, and archive contents covered by local
  checks before calling any surface release-ready.
- Treat external roadmap or audit notes as triage input, not scope expansion.
  Web middleware items such as JWT, CSRF, NestJS, rate limiting, and diagnostic
  routes are not part of this repository unless the project identity changes.
- Add one regression check for each fixed security, data-loss, format, or
  packaging bug.

Done when release-relevant docs and claims are checked by `release-preflight`,
and every active roadmap item maps to an existing SecureKit surface.

### 2. Add Release SBOM After Archive Contents Stabilize

Goal: let users inspect release contents without unpacking every archive.

- Add SBOM generation after archive contents stop changing.
- Document user verification steps in `docs/RELEASE_CHECKLIST.md`.

Done when release generation fails on missing SBOM output, and users can map
release assets to documented package contents.

## Next

### 3. Tighten Negative Compatibility Coverage

Goal: make malformed or hostile `SKT1`, `SKF1`, and `SKP1` inputs boringly
rejectable.

- Add negative fixtures only for specific uncovered `docs/FORMAT.md` rules.
- Keep authentication failures generic and indistinguishable where the security
  model requires it.
- Keep path-based file open behavior covered for no-overwrite, temp-file commit,
  and cleanup after authentication failure.
- Keep stream-output rollback limits documented and tested only where the API can
  enforce behavior.

Done when each new fixture names the format rule it protects, and the smallest
matching test fails without the fix.

### 4. Improve User Entry Points Only When Needed

Goal: make common use safer without expanding scope blindly.

- Add `CONTRIBUTING.md` only when outside contributors need one-command local
  checks.
- Add examples only when README recipes become too crowded.
- Add CLI `inspect` or `verify` only after operators provide real use cases.
- Add package-manager recipes only after release archives are validated.

Done when each addition has a concrete user need, one runnable check, and no
security-model contradiction.

## Parked

These stay blocked until the gate is met:

- Result-style APIs: two real call sites show exceptions are the wrong boundary.
- Object-oriented APIs beyond packet streaming: two real call sites duplicate
  lifecycle logic that free functions cannot express cleanly.
- New password-file KDF profile: format spec, downgrade behavior, bounds,
  fixture policy, and at least three known-answer vectors.
- Additional streaming format: written threat model for plaintext-before-auth
  and output ownership.
- OpenSSL provider or FIPS helpers: documented support policy and dedicated
  tests.
- Scheduled long-running fuzz: corpus policy and useful smoke-target signal.
- Negative compatibility fixture expansion: specific uncovered `FORMAT.md`
  reject rule.
- CLI split: repeated edit conflicts in `src/cli/main.cpp`.
- Benchmarks: correctness, format, and release gates stay stable first.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Web framework middleware, including Express, Koa, Fastify, NestJS, JWT, CSRF,
  CORS, rate limiting, request validation, or diagnostic web routes.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
