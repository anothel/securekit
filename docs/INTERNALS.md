# SecureKit Internal Boundaries

This document records current implementation ownership. It is not a public API
contract.

## Current Ownership

- `src/aead.cpp`: one-shot `SKT1` packet encryption/decryption and OpenSSL
  AES-256-GCM helpers shared by packet APIs.
- `src/packet_stream.cpp`: move-only `SKT1` packet streaming state machines.
  `packet_decryptor::update()` returns unverified plaintext until `finalize()`
  succeeds.
- `src/file.cpp`: public file API orchestration and stream payload flow for
  `SKF1` and `SKP1`.
- `src/file_detail.hpp`: private file-format, crypto, and temp-output boundary
  shared by the file implementation units.
- `src/file_crypto.cpp`: `SKF1`/`SKP1` header parse/serialize, file key
  derivation, password KDF checks, chunk nonce/AAD construction, and chunk
  AES-256-GCM.
- `src/file_io.cpp`: file input, exclusive temp-output creation, durable commit,
  cleanup, and stream read/write helpers.
- `src/cli/main.cpp`: CLI executable entry point.
- `src/cli/commands.cpp`: CLI parsing, command dispatch, help text, exit-code
  policy, file command verification, and user-facing diagnostics.
- `tests/fixtures`: stable byte-level compatibility vectors and negative
  fixtures for documented reject rules.

## Split Gates

Internal splits are allowed only when they reduce active maintenance risk. They
must not change public C++ APIs, CLI command shape, or `SKT1`/`SKF1`/`SKP1`
format behavior.

Keep file and CLI internals private. Further splits require repeated edit
conflicts, a smaller safety fix, or a test-protected behavior change that is
hard to review in the current files.

## Required Checks

Any internal split must run:

```sh
cmake --build build --config Release --target check
cmake --build build --config Release --target release-preflight
```

If the split touches package installation, CLI usage, release assets, or public
headers, run the smallest matching package or CLI check before
`release-preflight`.
