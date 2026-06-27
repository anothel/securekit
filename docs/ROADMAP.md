# SecureKit Roadmap

Future work only. Completed work belongs in Git history and release notes, not a
standing changelog.

## Rules

1. Security, data loss, and format compatibility come first.
2. Keep v0.1.x public API changes minimal and keep the v1 free-function API
   stable unless real call sites prove otherwise.
3. Keep OpenSSL as the crypto backend.
4. Keep CMake install/export, the CLI, and GitHub Actions package checks as
   first-class release surfaces.
5. Do not add public API, wire formats, package channels, CI cost, or release
   ceremony without a written problem, regression check, and rollback plan.
6. Active roadmap items must name an existing SecureKit surface.

Release-impacting work must pass the matching configured build directory:

```sh
cmake --build build --config Release --target release-preflight
```

## Now

### Prepare v0.2.0 release candidate

Surface: CLI, release assets, package checks, docs.

Problem:
`main` contains post-v0.1.0 user-facing CLI verification commands, checked
examples, negative compatibility coverage, and release provenance/SBOM work
that are not yet shipped in a release. This is release closure work, not a
reason to add more features.

Checks:

- `release-preflight`
- SecureKit CI
- Linux sanitizer job
- macOS package-check
- Windows package-check
- CodeQL
- README, `FORMAT.md`, `SECURITY_MODEL.md`, and release checklist match
  behavior

Rollback:
Delay the tag, or revert/postpone the CLI verification commands if the surface
is not ready to support.

Done:
v0.2.0 is tagged and published with release notes covering:

- `verify-file` / `verify-file-password`
- checked basic example
- negative compatibility fixture hardening
- release SBOM / provenance attestation changes

## Next

### Dogfood SecureKit in one real consumer

Surface: C++ API, installed CMake package, CLI.

Problem:
The project needs real call-site pressure before adding Result-style APIs, OO
APIs, new KDF profiles, streaming formats, or package channels.

Check:

- Build one small external consumer from the installed package or release
  archive.
- Use at least: `keygen`, `seal-file` / `open-file`, `verify-file`, password
  file flow, and one C++ API path.
- Record repeated friction as concrete issues with command/API examples.

Done:
Either no new surface is needed, or a parked item has a proven gate and can move
to `Next`.

## Parked

These are blocked candidates, not queued work. Move one to `Now` or `Next` only
after its gate is proven and a runnable check is named.

- Result-style APIs: gate is two real call sites showing exceptions are the
  wrong boundary.
- Object-oriented APIs beyond packet streaming: gate is two real call sites
  duplicating lifecycle logic that free functions cannot express cleanly.
- New password-file KDF profile: gate is format spec, downgrade behavior,
  bounds, fixture policy, and at least three known-answer vectors.
- Additional streaming format: gate is a written threat model for
  plaintext-before-auth and output ownership.
- OpenSSL provider or FIPS helpers: gate is documented support policy and
  dedicated tests.
- Scheduled long-running fuzz: gate is corpus policy and useful smoke-target
  signal.
- Further negative compatibility fixture expansion: gate is a specific
  uncovered `FORMAT.md` reject rule found by comparing `docs/FORMAT.md` with
  `tests/fixtures/negative/README.md`.
- CLI split: gate is repeated edit conflicts in `src/cli/main.cpp`.
- Benchmarks: gate is stable correctness, format, and release checks.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Web framework middleware, including Express, Koa, Fastify, NestJS, JWT, CSRF,
  CORS, rate limiting, request validation, or diagnostic web routes.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
