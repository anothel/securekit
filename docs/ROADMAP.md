# SecureKit Roadmap

This file tracks only future work. Completed work belongs in Git history.

## Rules

1. Security, data loss, and format compatibility come before convenience.
2. Keep the v1 free-function API stable unless real call sites prove otherwise.
3. Keep OpenSSL as the crypto backend.
4. Keep CMake install/export and the CLI as first-class package surfaces.
5. Do not add public API, formats, package channels, or CI cost without a written
   problem, regression check, and rollback plan.

Release-impacting work must pass:

```sh
cmake --build build --config Release --target release-preflight
```

Use the matching configured build dir on Windows or in CI.

## Next Work

### 1. Cut A Release Candidate

Goal: make the next tag a mechanical release, not a judgment call.

Tasks:

1. Pick the next SemVer version.
2. Make `CMakeLists.txt`, README examples, package artifact names, release notes,
   and tag name agree.
3. Run `release-preflight`.
4. Confirm required GitHub Actions jobs are green.
5. Confirm staged release assets include binary archives, source archives, and
   `SHA256SUMS.txt`.

Done when:

- `release-preflight` passes.
- Required CI failures: 0.
- Package artifact version mismatches: 0.
- Documented release commands point to existing targets.

### 2. Keep Security Docs Matching Code

Goal: prevent docs from promising more than code provides.

Tasks:

1. Keep `SECURITY.md` current with private reporting path, supported versions,
   disclosure scope, and in-scope security surfaces.
2. Keep `docs/FORMAT.md` current with `SKT1`, `SKF1`, and `SKP1` byte layouts,
   IDs, AAD rules, record limits, and reject rules.
3. Keep `docs/SECURITY_MODEL.md` current with threat model, non-goals,
   plaintext-before-auth rules, file output safety, password KDF policy,
   memory-wipe limits, and error-message policy.
4. Keep `docs/KDF_AGILITY.md` as the gate for any future password-file KDF
   profile.

Done when:

- Public docs contradict `include/`, `src/`, tests, or fixtures: 0.
- Serialized format fields missing from `docs/FORMAT.md`: 0.
- Docs claim guaranteed file durability or guaranteed memory erasure: 0.

### 3. Add Supply-Chain Trust After Release Shape Stabilizes

Goal: let users verify release artifact origin, not only checksums.

Tasks:

1. Sign or attest `SHA256SUMS.txt`.
2. Add provenance attestation if GitHub Actions release flow stays stable.
3. Add SBOM generation after archive contents stop changing.
4. Document user verification steps in the release checklist.

Done when:

- Release fails if signing/provenance generation fails.
- Users can verify artifact integrity and origin from documented commands.
- SBOM covers shipped source and binary package contents.

### 4. Improve User Entry Points Only When Needed

Goal: make common use safer without expanding scope blindly.

Candidates:

- `CONTRIBUTING.md` when outside contributors need one-command local checks.
- Examples directory when README recipes become too crowded.
- CLI `inspect`/`verify` when operators provide real use cases.
- Package-manager recipes after a release archive is validated.

Done when each candidate has:

- concrete user or maintainer need;
- smallest runnable check;
- no contradiction with the security model.

## Gated Ideas

These stay parked until the gate is met:

- Result-style APIs: two real call sites must show exceptions are the wrong
  boundary.
- Object-oriented APIs beyond packet streaming: two real call sites must
  duplicate lifecycle logic that free functions cannot express cleanly.
- New password-file KDF profile: format spec, downgrade behavior, bounds,
  fixture policy, and at least three known-answer vectors first.
- Additional streaming format: written threat model for plaintext-before-auth
  and output ownership first.
- OpenSSL provider or FIPS helpers: documented support policy and dedicated
  tests first.
- Scheduled long-running fuzz: corpus policy and useful smoke-target signal
  first.
- Negative compatibility fixture expansion: specific uncovered FORMAT reject
  rule first.
- CLI split: repeated edit conflicts in `src/cli/main.cpp` first.
- Benchmarks: correctness, format, and release gates stay stable first.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
