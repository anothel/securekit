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

### 1. Add Supply-Chain Trust After Release Shape Stabilizes

Goal: let users verify release artifact origin, not only checksums.

- Sign or attest `SHA256SUMS.txt`.
- Add provenance attestation if the GitHub Actions release flow stays stable.
- Add SBOM generation after archive contents stop changing.
- Document user verification steps in `docs/RELEASE_CHECKLIST.md`.

Done when release generation fails on missing signing/provenance, and users can
verify artifact integrity and origin from documented commands.

## Next

### 2. Improve User Entry Points Only When Needed

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
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
