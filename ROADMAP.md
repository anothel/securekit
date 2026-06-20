# SecureKit Roadmap

This file tracks forward-looking work only. Completed migration, CI, package,
and handoff notes belong in Git history.

## Direction

- Keep the v1 free-function API stable unless real call sites prove a change is
  needed.
- Keep OpenSSL as the crypto backend.
- Keep CMake install/export and the CLI as first-class package surfaces.
- Add compatibility vectors and hardening tests when behavior changes or risk is
  found.
- Avoid framework-scale abstractions without call-site pressure.

## Release Readiness Bar

Every roadmap item should leave the project in a releasable state. A task is not
done until its related local checks pass and the public docs match the shipped
surface.

Minimum local verification for release-impacting work:

```sh
cmake --build build --config Release --target check
cmake --build build --config Release --target package-check
cmake --build build --config Release --target release-workflow-check
```

Use the equivalent configured build directory on Windows or in CI.

## Milestone 1: Public Surface Audit

Goal: make the public API, README examples, headers, CLI help, and consumer
smoke tests agree exactly.

Scope:

- Compare every declaration in `include/securekit/*.hpp` with the README Public
  API section.
- Compare every CLI command and option in `src/cli/main.cpp` help text with the
  README CLI examples.
- Ensure `tests/public_headers_test.cpp` includes all public headers.
- Ensure `tests/consumer/main.cpp` exercises each public module at least once.

Exit criteria:

- Zero known README/header API drift.
- Zero known README/CLI help drift.
- All public headers compile from a consumer translation unit.
- Package consumer still builds and runs from an installed package.

Verification:

```sh
cmake --build build --config Release --target check
cmake --build build --config Release --target package-check
```

## Milestone 2: CLI Error Contract Audit

Goal: make command failures predictable enough for scripts.

Scope:

- Inventory all CLI commands and failure classes.
- Keep successful text-output commands to `stdout` plus one trailing newline.
- Keep parse, usage, file, decode, and authentication failures on `stderr` with
  non-zero exit codes.
- Add or adjust CLI tests where a documented failure class lacks coverage.

Exit criteria:

- Each command has success coverage.
- Each command family has representative parse and input-source conflict
  coverage.
- File-producing commands keep overwrite refusal coverage.
- Authenticated commands keep wrong key/password/AAD coverage.

Verification:

```sh
cmake --build build --config Release --target check
```

## Milestone 3: Compatibility Vector Policy

Goal: make wire-format compatibility changes deliberate and reviewable.

Scope:

- Keep fixture-backed coverage for `SKT1`, key wrapping, `SKF1`, and `SKP1`.
- Add a new known-answer fixture for every intentional wire-format behavior
  change.
- Keep fixture inventory tests strict enough to catch missing or stale fixture
  files.
- Document any future format revision before adding implementation code.

Exit criteria:

- Every supported serialized format has at least one positive fixture-backed
  round trip.
- Every supported serialized format has representative malformed/truncated
  rejection coverage.
- Fixture filenames and README compatibility notes stay aligned.

Verification:

```sh
cmake --build build --config Release --target check
```

## Milestone 4: Package Matrix Parity

Goal: keep package behavior consistent across local builds and GitHub Actions.

Scope:

- Keep install-only, static-library, shared-library, and default package checks
  aligned with CI.
- Keep source archives buildable after extraction.
- Keep release artifact naming, source archive de-duplication, and checksum
  generation covered by workflow checks.
- Re-run the matrix after changes to CMake install/export, public headers, CLI
  installation, or CPack settings.

Exit criteria:

- `package-check` installs, tests the installed CLI, builds a consumer, creates
  binary and source archives, checks archive members, extracts a source archive,
  builds it, installs it, and runs `securekit --version`.
- CI package jobs upload artifacts from every package-check job.
- Release workflow rejects tag/package version mismatches.

Verification:

```sh
cmake --build build --config Release --target package-check
cmake --build build --config Release --target release-workflow-check
```

## Milestone 5: Security Boundary Review

Goal: make documented security boundaries match implementation and tests.

Scope:

- Review file output atomicity and cleanup on every newly discovered failure
  path.
- Review packet and file decrypt behavior where plaintext can appear before
  authentication completes.
- Keep OpenSSL provider, backend error, password KDF, and key-erasure limits
  documented.
- Add focused tests for any newly discovered boundary condition.

Exit criteria:

- No known undocumented security boundary.
- No known mismatch between documented boundary and implementation behavior.
- New boundary fixes include regression tests.

Verification:

```sh
cmake --build build --config Release --target check
```

## Parking Lot

These are intentionally not scheduled until real call sites justify them:

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
