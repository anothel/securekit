# SecureKit Roadmap

This file tracks forward-looking work only. Completed CLI contract,
compatibility-vector, package-matrix, release-preflight, security-boundary,
public API/consumer freeze, CI hardening, usage/package polish, and
documentation-consistency hardening work belongs in Git history, not in the
active roadmap.

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

External analyses are reduced to roadmap-impacting work only. Items already
enforced by tests, docs, or release targets are not repeated as active work.

Accepted from the 2026-06-22 analysis:

- security, changelog, format, and security-model docs
- stronger streaming decryptor warning

Accepted from the 2026-06-24 code-based reanalysis:

- fuzz scaffolding for strict decoders, packet parsers, and file parsers
- file commit durability as a separate question from no-overwrite atomicity
- best-effort internal zeroization for derived keys and temporary plaintext
- stronger `constant_time_equal` equal-length precondition documentation
- KDF agility as a design-and-fixture task before any new `SKP1` behavior

Already covered by the current tree:

- release preflight and release workflow shape checks
- compatibility fixtures for `SKT1`, key wrap, `SKF1`, and `SKP1`
- package-check install, archive, source-build, and consumer coverage
- CLI error contract and public header/package consumer checks
- CI hardening through warnings-as-errors, sanitizer, and macOS package jobs
- task-oriented CLI recipes, package/runtime version API, and consumer CMake
  option split
- strict hex/base64/base64url rejection rules
- chunked file sealing, temp-output cleanup, no-overwrite path commits, and
  `SKP1` fixed-parameter scrypt docs
- SKF1/SKP1 1 MiB boundaries, malformed record shape, trailing data,
  unsupported scrypt params, and packet prefix/tag truncation tests

Deferred or rejected items are kept only in the Parking Lot or Not Planned
sections below, with the gating reason next to each item.

## Active Work Order

### 1. Documentation Baseline For v1

Goal: keep security, format, and release docs aligned with shipped behavior.

Tasks:

1.1. Keep `SECURITY.md` current with the private vulnerability reporting path,
report template, supported-version policy, disclosure scope, and in-scope
security surfaces.

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

### 2. Release Candidate Cut

Goal: make the next tag cut a mechanical operation instead of a judgment call.

Tasks:

2.1. Pick the next SemVer version and make `CMakeLists.txt` project version,
release tag, package artifact names, README examples, and changelog agree.

2.2. Run the local release preflight from a clean build directory:

```sh
cmake --build build --config Release --target release-preflight
```

2.3. Confirm GitHub Actions has zero failing required jobs on the release
candidate commit.

2.4. Confirm `release-preflight` staged local release assets and checksums:

- At least one binary package artifact exists.
- At least one source package artifact exists.
- `SHA256SUMS.txt` covers the staged archives.

Exit criteria:

- `release-preflight` passes, including `check`, `package-check`, and
  `release-workflow-check`.
- 0 failing required CI jobs.
- 0 package artifact version/tag mismatches.
- 0 README or release checklist commands that point to missing targets.

### 3. Code-Based Hardening Backlog

Goal: convert the 2026-06-24 code-based reanalysis into small, verified
hardening slices after the release candidate work.

Tasks:

3.1. Add a fuzz scaffold without changing shipped APIs:

- `SECUREKIT_BUILD_FUZZ=ON` config path.
- At least five targets covering hex, base64, base64url, `SKT1` packet parsing,
  and `SKF1`/`SKP1` file header or open paths.
- Seed corpus reuses `tests/fixtures` plus malformed minimal samples.
- Short smoke command documented; long fuzz remains manual or scheduled.

3.2. Reduce streaming decrypt misuse risk without redesigning the API first:

- Keep the low-level `packet_decryptor` API.
- Add a misuse-focused test or README example showing that bytes returned from
  `update()` are discarded unless `finalize()` succeeds.
- Add a higher-level verified wrapper only if real call sites need streaming
  output ownership beyond existing one-shot `decrypt()`.

3.3. Decide and implement file commit durability policy:

- Document the difference between no-overwrite atomicity and power-loss
  durability.
- If enabled, flush temp file data before commit and flush parent directory or
  platform equivalent where practical.
- Preserve existing refusal to overwrite output paths.

3.4. Add best-effort internal zeroization only where SecureKit owns the buffer:

- Start with derived file keys, unwrap temporary plaintext, and temporary
  decrypt plaintext.
- Use a small internal wipe helper; do not introduce public allocator types
  without measured need.
- Keep the public non-goal: no guaranteed portable key erasure.

3.5. Design KDF agility before implementation:

- Written downgrade policy.
- Supported profile IDs or new format version decision.
- Memory/time upper bounds.
- At least three old/new compatibility vectors before code accepts new params.

Exit criteria:

- New hardening code has the smallest runnable check that would fail if it
  regresses.
- `release-preflight` still passes after release-impacting changes.
- No new public API unless the task names the call-site pressure it solves.
- Docs do not claim stronger durability or memory erasure than the code
  provides.

### 4. Deferred Feature Intake Rules

Goal: keep parked ideas from turning into speculative scope.

Tasks:

4.1. Non-throwing result-style APIs stay deferred until at least two real call
sites show exception handling is the wrong boundary.

4.2. Object-oriented APIs beyond the packet streaming objects stay deferred until
at least two real call sites duplicate lifecycle logic that free functions cannot
express cleanly.

4.3. Additional password formats or KDF agility implementation stays deferred
until there is a written format spec, fixed downgrade behavior, fixture policy
updates, and at least three known-answer vectors for the new format.

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
- Additional password formats or KDF agility implementation before the design
  and fixture gate above is met.
- Additional streaming formats beyond `SKT1`.
- Explicit OpenSSL provider or FIPS configuration helpers.
- CLI `inspect` and `verify` commands until operational use cases are written.
- Package-manager recipes until a release archive has been validated.
- SBOM, provenance, and signing until release artifact shape is stable.
- Examples directory until use cases outgrow README recipes.
- `CONTRIBUTING.md` until release-critical docs and CI are settled.
- Benchmarks until correctness, format, and CI hardening work is stable.
- CLI `main.cpp` split until command behavior churn creates repeated edit
  conflicts.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
