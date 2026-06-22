# SecureKit Roadmap

This file tracks forward-looking work only. Completed CLI contract,
compatibility-vector, package-matrix, release-preflight, security-boundary,
public API/consumer freeze, and documentation-consistency hardening work belongs
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
cmake --build build --config Release --target release-preflight
```

Use the equivalent configured build directory on Windows or in CI.

## Analysis Intake Summary

The 2026-06-22 improvement analysis has been reduced to roadmap-impacting work.
Items already enforced by tests or release targets are not repeated as active
work.

Accepted from the analysis:

- security, changelog, format, and security-model docs
- stronger streaming decryptor warning
- sanitizer and macOS CI coverage
- task-oriented CLI recipes
- package/runtime version API
- consumer-facing CMake option split

Already covered by the current tree:

- release preflight and release workflow shape checks
- compatibility fixtures for `SKT1`, key wrap, `SKF1`, and `SKP1`
- package-check install, archive, source-build, and consumer coverage
- CLI error contract and public header/package consumer checks

Deferred or rejected items are kept only in the Parking Lot or Not Planned
sections below, with the gating reason next to each item.

## Active Work Order

### 1. Documentation Baseline For v1

Goal: keep security, format, and release docs aligned with shipped behavior.

Tasks:

1.1. Keep `SECURITY.md` current with the vulnerability reporting path,
supported-version policy, disclosure scope, and in-scope security surfaces.

1.2. Keep `docs/FORMAT.md` current with `SKT1`, `SKF1`, and `SKP1` byte
layouts, IDs, AAD rules, record limits, and compatibility rules.

1.3. Keep `docs/SECURITY_MODEL.md` current with threat model, non-goals,
plaintext-before-auth rules, file output safety, password KDF policy, memory
limits, and error-message policy.

1.4. Keep `CHANGELOG.md` updated before release tags.

Exit criteria:

- 0 public docs that contradict `include/`, `src/`, tests, or fixtures.
- 0 undocumented serialized format fields.
- 0 release checklist items that require a missing local command.
- `git diff --check` passes after doc edits.

### 2. CI Hardening

Goal: catch platform and memory-undefined behavior issues before release.

Tasks:

2.1. Add `SECUREKIT_WARNINGS_AS_ERRORS`.

2.2. Add a Linux sanitizer CI job with AddressSanitizer and
UndefinedBehaviorSanitizer.

2.3. Add a macOS package-check job with AppleClang and Homebrew OpenSSL 3.

Exit criteria:

- sanitizer CI runs `check`.
- macOS CI runs `check` and `package-check`.
- new CI jobs do not duplicate every existing matrix row.

### 3. Usage And Package Polish

Goal: make common consumer and CLI use easier without expanding crypto scope.

Tasks:

3.1. Add task-oriented CLI recipes for file sealing/opening, AAD use, password
files, stdout/stderr behavior, exit codes, and automation examples.

3.2. Add `securekit::version()` and numeric version helpers if they can be kept
header/runtime consistent with the CMake project version.

3.3. Split CMake options only where consumers get immediate value:
`SECUREKIT_BUILD_CLI`, `SECUREKIT_INSTALL_CLI`, and
`SECUREKIT_WARNINGS_AS_ERRORS`.

Exit criteria:

- CLI recipes do not duplicate or contradict README command contracts.
- Version API matches `securekit --version` and CMake package version.
- Library-only consumers can configure without building or installing the CLI.

### 4. Release Candidate Cut

Goal: make the next tag cut a mechanical operation instead of a judgment call.

Tasks:

4.1. Pick the next SemVer version and make `CMakeLists.txt` project version,
release tag, package artifact names, README examples, and changelog agree.

4.2. Run the local release preflight from a clean build directory:

```sh
cmake --build build --config Release --target release-preflight
```

4.3. Confirm GitHub Actions has zero failing required jobs on the release
candidate commit.

4.4. Inspect package-check artifacts and confirm:

- At least one binary package artifact exists.
- At least one source package artifact exists.
- Extracted source configures, builds, installs, and runs `securekit --version`.

Exit criteria:

- `release-preflight` passes, including `check`, `package-check`, and
  `release-workflow-check`.
- 0 failing required CI jobs.
- 0 package artifact version/tag mismatches.
- 0 README or release checklist commands that point to missing targets.

### 5. Deferred Feature Intake Rules

Goal: keep parked ideas from turning into speculative scope.

Tasks:

5.1. Non-throwing result-style APIs stay deferred until at least two real call
sites show exception handling is the wrong boundary.

5.2. Object-oriented APIs beyond the packet streaming objects stay deferred until
at least two real call sites duplicate lifecycle logic that free functions cannot
express cleanly.

5.3. Additional password formats or KDF agility stay deferred until there is a
written format spec, fixed downgrade behavior, fixture policy updates, and at
least three known-answer vectors for the new format.

5.4. Additional streaming formats beyond `SKT1` stay deferred until a written
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
- CLI `inspect` and `verify` commands until operational use cases are written.
- Fuzz targets until sanitizer CI exists.
- Package-manager recipes until a release archive has been validated.
- SBOM, provenance, and signing until release artifact shape is stable.
- Examples directory until CLI recipes exist.
- `CONTRIBUTING.md` until release-critical docs and CI are settled.
- Benchmarks until correctness, format, and CI hardening work is stable.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
