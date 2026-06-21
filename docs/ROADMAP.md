# SecureKit Roadmap

This file tracks forward-looking work only. Completed CLI contract,
compatibility-vector, package-matrix, and security-boundary hardening work belongs
in Git history, not in the active roadmap.

## Direction

1. Keep the v1 free-function API stable unless real call sites prove a change is
   needed.
2. Keep OpenSSL as the crypto backend.
3. Keep CMake install/export and the CLI as first-class package surfaces.
4. Add compatibility vectors and hardening tests when behavior changes or risk is
   found.
5. Avoid framework-scale abstractions without call-site pressure.

## Release Readiness Bar

Every active roadmap item must leave the project in a releasable state. A task is
not done until its related local checks pass and public docs match the shipped
surface.

Minimum local verification for release-impacting work:

```sh
cmake --build build --config Release --target check
cmake --build build --config Release --target package-check
cmake --build build --config Release --target release-workflow-check
```

Use the equivalent configured build directory on Windows or in CI.

## Active Work Order

### 1. Release Candidate Preflight

Goal: make the next tag cut a mechanical operation instead of a judgment call.

Tasks:

1.1. Pick the next SemVer version and make `CMakeLists.txt` project version,
release tag, package artifact names, and README examples agree.

1.2. Run these three local targets from a clean build directory:

```sh
cmake --build build --config Release --target check
cmake --build build --config Release --target package-check
cmake --build build --config Release --target release-workflow-check
```

1.3. Confirm GitHub Actions has zero failing required jobs on the release
candidate commit.

1.4. Inspect package-check artifacts and confirm:

- At least one binary package artifact exists.
- At least one source package artifact exists.
- Extracted source configures, builds, installs, and runs `securekit --version`.

Exit criteria:

- 3/3 local release preflight targets pass.
- 0 failing required CI jobs.
- 0 package artifact version/tag mismatches.
- 0 README or release checklist commands that point to missing targets.

### 2. Public API and Consumer Freeze Audit

Goal: make the v1 shipped surface explicit before adding new features.

Tasks:

2.1. Compare every public header under `include/securekit/` with the README
`Public API` section.

2.2. Confirm the installed-package consumer covers all six public API families:

- Utility codecs and hashing.
- Random and token generation.
- One-shot `SKT1` packet encryption/decryption.
- Streaming `SKT1` packet encryption/decryption.
- Key wrapping.
- `SKF1` and `SKP1` file APIs.

2.3. If the audit finds drift, fix docs or consumer coverage. Do not add public
API during this task.

Exit criteria:

- 0 exported public functions or classes missing from README.
- 0 README public API entries missing from headers.
- 6/6 public API families covered by the consumer check.
- `check` passes.
- `package-check` passes if the consumer or install surface changes.

### 3. Documentation Consistency Audit

Goal: keep README, release docs, fixture policy, and roadmap aligned with the
current shipped behavior.

Tasks:

3.1. Compare README CLI examples against `securekit help` and command-specific
usage for every supported command family.

3.2. Compare README release instructions with `docs/RELEASE_CHECKLIST.md`,
`.github/workflows/securekit-ci.yml`, and CMake targets.

3.3. Compare compatibility fixture policy in `tests/fixtures/README.md` with the
fixture inventory tests.

3.4. Remove completed roadmap items instead of carrying them forward.

Exit criteria:

- 0 stale CLI examples.
- 0 stale release commands.
- 0 fixture policy/test mismatches.
- 0 completed items left in the active roadmap.

### 4. Deferred Feature Intake Rules

Goal: keep parked ideas from turning into speculative scope.

Tasks:

4.1. Non-throwing result-style APIs stay deferred until at least two real call
sites show exception handling is the wrong boundary.

4.2. Object-oriented APIs beyond the packet streaming objects stay deferred until
at least two real call sites duplicate lifecycle logic that free functions cannot
express cleanly.

4.3. Additional password formats or KDF agility stay deferred until there is a
written format spec, fixed downgrade behavior, fixture policy updates, and at
least three known-answer vectors for the new format.

4.4. Additional streaming formats beyond `SKT1` stay deferred until a written
threat model explains plaintext-before-auth handling and output ownership.

Exit criteria:

- Each accepted deferred feature has a written problem statement.
- Each accepted deferred feature has concrete call-site evidence.
- Each accepted cryptographic format change has compatibility vectors before
  implementation code.

## Parking Lot

These are intentionally unscheduled until the intake rules above are met:

- Object-oriented APIs beyond the existing packet streaming objects.
- Non-throwing result-style APIs.
- Additional password formats or KDF agility.
- Additional streaming formats beyond `SKT1`.
- Explicit OpenSSL provider or FIPS configuration helpers.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
