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

## Imported Improvement Analysis, 2026-06-22

The user-provided improvement analysis is tracked here instead of being silently
dropped. Disposition meanings:

- `Accepted`: planned or now added to the active work order.
- `Already Done`: verified in the current tree; keep tests/docs guarding it.
- `Deferred`: useful, but blocked on a smaller prerequisite or real call-site
  evidence.
- `Not Planned`: intentionally outside SecureKit's current scope.

| Item | Disposition | Reason |
| --- | --- | --- |
| `SECURITY.md` | Accepted | Required before public security reports are expected. |
| `CHANGELOG.md` | Accepted | Required before version tags are cut. |
| `docs/FORMAT.md` for `SKT1`, `SKF1`, `SKP1` | Accepted | Serialized formats need one compatibility reference outside README. |
| `docs/SECURITY_MODEL.md` | Accepted | Security boundaries need one stable document outside README. |
| Stronger streaming decryptor warning | Accepted | `packet_decryptor::update()` returns plaintext before tag verification. |
| Release candidate checklist/preflight | Already Done | `release-preflight` now runs local release checks and artifact validation. |
| Compatibility fixtures | Already Done | `tests/fixtures` pins `SKT1`, key wrap, `SKF1`, and `SKP1` vectors. |
| Package consumer/install checks | Already Done | `package-check` builds archives, installs, runs CLI, and builds a consumer. |
| CLI error contract hardening | Already Done | CLI tests cover usage, parse, auth, overwrite, stdout/stderr cases. |
| Public API/consumer freeze | Already Done | Public headers and package consumer tests guard the shipped surface. |
| Sanitizer CI | Accepted | High-value C++ hardening; keep scope to one Linux sanitizer job first. |
| macOS CI | Accepted | Platform coverage gap for CMake/OpenSSL/AppleClang consumers. |
| CLI recipes document | Accepted | README has command catalog; recipes should become task-oriented docs. |
| `securekit::version()` API | Accepted | Useful for package/runtime diagnostics and aligns with CLI version output. |
| CMake options split | Accepted | Consumers may need library-only builds and stricter local policy toggles. |
| Fuzz/property tests | Deferred | Do after sanitizer CI so parser fuzz failures get stronger diagnostics. |
| CLI `inspect` | Deferred | Needs written operational use cases and sensitive-output contract first. |
| CLI `verify` | Deferred | Needs clear no-plaintext-output contract and file/stream behavior spec first. |
| Password/KDF agility | Deferred | Requires spec, downgrade policy, and fixtures before implementation. |
| Package-manager recipes | Deferred | Do after release archive/install flow is validated by a tagged release. |
| SBOM/provenance/signing | Deferred | Useful after release artifact shape is stable. |
| Examples directory | Deferred | Add after CLI recipes so examples do not duplicate or contradict docs. |
| `CONTRIBUTING.md` | Deferred | Lower risk than security/format/CI work before v1. |
| Benchmarks | Deferred | Optimize only after correctness, formats, and CI hardening are stable. |
| Custom crypto primitives | Not Planned | SecureKit wraps OpenSSL; no homegrown primitives. |
| Caller-selected nonces | Not Planned | Public nonce control increases misuse risk in v1. |
| Generic crypto framework | Not Planned | Conflicts with small utility-library scope. |
| TLS/network features | Not Planned | Out of scope; callers should use TLS libraries. |
| Password hashing library | Not Planned | `SKP1` file encryption only; account password hashing is a different product. |
| Guaranteed secure erasure claims | Not Planned | Not portable with standard containers, allocators, swap, and crash dumps. |

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

### 3. Release Candidate Cut

Goal: make the next tag cut a mechanical operation instead of a judgment call.

Tasks:

3.1. Pick the next SemVer version and make `CMakeLists.txt` project version,
release tag, package artifact names, README examples, and changelog agree.

3.2. Run the local release preflight from a clean build directory:

```sh
cmake --build build --config Release --target release-preflight
```

3.3. Confirm GitHub Actions has zero failing required jobs on the release
candidate commit.

3.4. Inspect package-check artifacts and confirm:

- At least one binary package artifact exists.
- At least one source package artifact exists.
- Extracted source configures, builds, installs, and runs `securekit --version`.

Exit criteria:

- `release-preflight` passes, including `check`, `package-check`, and
  `release-workflow-check`.
- 0 failing required CI jobs.
- 0 package artifact version/tag mismatches.
- 0 README or release checklist commands that point to missing targets.

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
- CLI `inspect` and `verify` commands until operational use cases are written.
- Fuzz targets until sanitizer CI exists.
- Package-manager recipes until a release archive has been validated.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
