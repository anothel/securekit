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

## External Analysis Triage

Broad audit documents must be split before implementation. Do the smallest
repo-relevant work first, then leave the rest parked or not planned.

| Audit theme | SecureKit handling | Required check |
| --- | --- | --- |
| Docs and implementation mismatch | Keep README, `FORMAT.md`, `SECURITY_MODEL.md`, release checklist, release notes, and public headers aligned with the C++ library and CLI. | `release-preflight` |
| Public contract comments | Keep path open, stream rollback, password byte handling, decrypt authentication, HKDF output size, and constant-time length caveats visible from public headers. | `public_headers_test`, `release-preflight` |
| Package and release trust | Keep install/export/package archives, release assets, checksums, SBOM, and provenance wiring checked before release. | `package-check`, `release-workflow-check`, `release-preflight` |
| Test gates and negative cases | Add regression coverage only for an existing SecureKit API, CLI, format, package, or release bug. | Smallest matching unit, CLI, fixture, or package check |
| Internal boundary pressure | Keep `src/file.cpp` and `src/cli/main.cpp` split gates documented before refactoring. | `docs/INTERNALS.md`, `release-preflight` |
| Fuzz and parser hardening | Keep fuzz smoke optional and corpus policy explicit; schedule long runs only with useful signal and an owner. | `fuzz-smoke` when configured |
| New API shape | Park until real call sites prove the need and name one runnable check. | Item-specific gate below |
| Web middleware findings | Not planned for this repository. | Roadmap scope guard |

The analyzed Node.js backend middleware backlog maps to Not Planned unless the
project identity changes: NestJS exports, Express/Koa/Fastify adapter parity,
CORS, CSRF, JWT, security event routing, diagnostics routes, sanitization,
file-upload validation, distributed rate limiting, public cache defaults, npm
package exports, `TEST_SUMMARY`, and CLI audit/config placeholders.

## Now

No active queued work. `dogfood-check` covers the v0.2.0 post-release
consumer loop and recorded no repeated friction. A parked item has a proven gate
only after that friction is recorded, and then can move to `Next`.

## Next

No queued feature work. This is a maintenance audit, not a feature start.
Promote one parked item only after dogfooding records real repeated friction,
proves the item's gate, and names one runnable check.

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
- `src/file.cpp` internal split: gate is repeated local edit pressure or safety
  work that is simpler after separating format parse/serialize, KDF, chunk
  AEAD, temp-file commit, and password header handling.
- CLI split: gate is repeated edit conflicts in `src/cli/main.cpp`.
- README split into `docs/CLI.md` or `docs/API.md`: gate is repeated reader or
  edit friction, not file length alone.
- Package-manager recipes: gate is validated release archives plus a real
  consumer request for a package-channel recipe.
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
