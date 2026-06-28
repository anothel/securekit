# SecureKit Internal Boundaries

This document records current implementation ownership. It is not a public API
contract.

## Current Ownership

- `src/aead.cpp`: one-shot `SKT1` packet encryption/decryption and OpenSSL
  AES-256-GCM helpers shared by packet APIs.
- `src/packet_stream.cpp`: move-only `SKT1` packet streaming state machines.
  `packet_decryptor::update()` returns unverified plaintext until `finalize()`
  succeeds.
- `src/file.cpp`: `SKF1` and `SKP1` file format parsing/serialization, file key
  derivation, chunk AEAD, path open temp-file commit, stream open behavior, and
  password header handling.
- `src/cli/main.cpp`: CLI parsing, command dispatch, help text, exit-code
  policy, file command verification, and user-facing diagnostics.
- `tests/fixtures`: stable byte-level compatibility vectors and negative
  fixtures for documented reject rules.

## Split Gates

Internal splits are allowed only when they reduce active maintenance risk. They
must not change public C++ APIs, CLI command shape, or `SKT1`/`SKF1`/`SKP1`
format behavior.

`src/file.cpp` may be split after tests protect the current behavior and one of
these pressures exists:

- format parse/serialize changes are repeatedly mixed with file I/O changes;
- KDF or password header work requires local isolation;
- chunk AEAD changes become hard to review next to temp-file commit behavior;
- a safety fix is smaller with a private helper boundary.

Preferred private split names, if the gate is met:

- `file_format_internal.*`
- `file_io_internal.*`
- `file_crypto_internal.*`
- `password_kdf_internal.*`

`src/cli/main.cpp` may be split only after repeated edit conflicts or repeated
review friction in one command area. Do not split it just because it is long.

## Required Checks

Any internal split must run:

```sh
cmake --build build --config Release --target check
cmake --build build --config Release --target release-preflight
```

If the split touches package installation, CLI usage, release assets, or public
headers, run the smallest matching package or CLI check before
`release-preflight`.
