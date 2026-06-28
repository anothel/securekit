# SecureKit Roadmap

Future work only. Completed work belongs in Git history and release notes, not a
standing changelog.

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
7. External audit or roadmap notes are triage input only. Notes for Node.js
   backend middleware repositories, including `package.json`, npm,
   `TEST_SUMMARY`, Express, Koa, Fastify, NestJS, JWT, CSRF, CORS, rate
   limiting, request validation, diagnostic routes, or adapter parity, do not
   become SecureKit work unless this repository's identity changes.

Release-impacting work must pass the matching configured build directory:

```sh
cmake --build build --config Release --target release-preflight
```

## Now

No active queued work. `dogfood-check` covers the v0.2.0 post-release
consumer loop and recorded no repeated friction. A parked item has a proven gate
only after that friction is recorded, and then can move to `Next`.

## Next

No queued feature work. Promote one parked item only after dogfooding records
real repeated friction, proves the item's gate, and names one runnable check.

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
- Scheduled long-running fuzz: gate is repeated useful `fuzz-smoke` signal and
  an owner for the scheduled job.
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
