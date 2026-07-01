# SecureKit External Security Review Packet

Review timing: run after package publication, when the reviewed commit matches
released and attested source assets.

## Review Scope

- `SKT1` packet parser/reject rules.
- `SKF1`/`SKP1` file parsing.
- path output safety.
- stream/stdout output ownership.
- AAD handling.
- password-derived key handling.
- release/security-reporting docs.

## Inputs

- `SECURITY.md`
- `docs/SECURITY_MODEL.md`
- `docs/FORMAT.md`
- `docs/CLI.md`
- `docs/KDF_AGILITY.md`
- `docs/OPENSSL_POLICY.md`
- `tests/fixtures`
- `tests/fixtures/negative`
- `tests/fuzz/corpus`
- released `securekit-X.Y.Z-source.tar.gz`
- `SHA256SUMS.txt`
- release SPDX SBOM
- GitHub artifact attestations

## Out Of Scope

Do not turn framework, web, JWT, CSRF, CORS, rate-limiting, networking, TLS,
custom crypto primitive, key-storage, allocator, or guaranteed memory-erasure
findings into SecureKit work unless this repository's identity changes first.

## Finding Intake

Each accepted finding must name an affected API, CLI command, serialized format, CMake package surface, release asset, or security-reporting surface. Findings that do not map to one of those surfaces stay as triage notes.

Include:

- reviewed commit or release tag.
- affected surface.
- reproducer or exact malformed input.
- expected behavior.
- actual behavior.
- exploitability assumptions.
- whether plaintext, keys, passwords, or AAD may be exposed.

## Regression Gate

Each accepted fix must add or name at least one focused regression check before
`release-preflight`:

- unit test.
- negative fixture.
- minimized `tests/fuzz/corpus` seed.
- release/package/security-doc preflight check.

## Rollback

Rollback is the smallest revert that removes the finding fix while preserving
public C++ APIs, CLI command shape, and `SKT1`/`SKF1`/`SKP1` compatibility unless
an approved gate explicitly changes them.
