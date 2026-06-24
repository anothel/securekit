# SecureKit Security Model

## Goals

SecureKit provides small C++20 helpers for byte-oriented cryptographic tasks:

- strict binary encoders and decoders
- SHA-256, HMAC-SHA-256, and HKDF-SHA-256 wrappers
- constant-time comparison for equal-length secret values
- OpenSSL-backed AES-256-GCM packets
- OpenSSL-backed chunked file sealing
- password-based file sealing for `SKP1`

The project aims to make these operations hard to misuse while keeping the API
surface small.

## Non-Goals

SecureKit does not provide:

- custom cryptographic primitives
- TLS or networking
- secure key storage
- guaranteed memory erasure
- password hashing for account authentication
- general KDF agility or caller-selected algorithms
- caller-selected nonces
- protection from compromised hosts, filesystems, shells, or build tools

## Threat Model

SecureKit assumes attackers may read, modify, truncate, reorder, append, or
replace serialized `SKT1`, `SKF1`, and `SKP1` data. It also assumes attackers may
provide wrong AAD, wrong keys, wrong passwords, and malformed input.

SecureKit does not assume attackers can execute code inside the caller process
or compromise OpenSSL, the operating system, or the compiler.

## Key Material Handling

Caller-owned keys and passwords remain caller responsibility. SecureKit does not
store them beyond the operation scope intentionally, but portable guaranteed
erasure is not promised.

Internal derived keys and OpenSSL contexts are scoped to the operation. Returned
`std::vector<std::byte>` values, caller buffers, allocator copies, swap files,
page files, crash dumps, logs, and debugger memory are outside SecureKit's
control.

Do not advertise SecureKit as guaranteeing secure memory wiping.

## Randomness

SecureKit relies on OpenSSL random byte APIs for keys, tokens, packet nonces,
file salts, and file nonce prefixes. Randomness backend failures are reported as
`securekit::error_code::backend_failure`.

Callers cannot supply nonces in the public API. This avoids accidental nonce
reuse across the v1 surface.

## AEAD Authentication Rules

AEAD decryption succeeds only after tag verification succeeds.

For one-shot `decrypt`, plaintext is returned only after authentication.

For `packet_decryptor`, `update()` returns unverified plaintext before
`finalize(tag)` checks the authentication tag. Callers must buffer or otherwise
hold that plaintext in a non-observable location until `finalize(tag)` returns
successfully.

Wrong keys, wrong AAD, modified ciphertext, modified headers, and modified tags
must not be distinguishable through detailed public error messages.

## File Output Safety

Path-based `open_file` and `open_file_with_password` refuse to overwrite an
existing output path. They write a temporary file in the output directory and
rename it to the requested output only after the whole file authenticates.
Authentication failure should not leave the requested output path behind.
Path-based file APIs flush SecureKit-owned temporary output before committing it
and use platform commit syncing where practical. This improves crash resilience,
but SecureKit does not guarantee survival across power loss, filesystem bugs, or
storage-device failure. If a post-commit directory sync fails on a platform that
supports it, SecureKit reports `backend_failure` and the output path may already
exist.

Stream-based open APIs write to caller-provided streams. They cannot delete,
truncate, or roll back bytes already accepted by that stream. Callers using
streams must treat the output as committed only after the function returns
successfully.

CLI file commands inherit the same distinction:

- path output refuses overwrite and commits after success
- `--out -` writes to stdout and cannot be rolled back

## Password-Based Encryption

`SKP1` accepts password bytes exactly as supplied. SecureKit does not normalize
Unicode, trim whitespace, prompt interactively, read environment variables, or
define password quality rules.

Current `SKP1` uses fixed OpenSSL scrypt parameters:

- `N = 32768`
- `r = 8`
- `p = 1`
- `maxmem = 64 MiB`

Unsupported stored scrypt parameters are rejected instead of silently accepted.
Future KDF agility requires a format spec, downgrade policy, and compatibility
vectors before implementation. `docs/KDF_AGILITY.md` is the gate for future
password-file KDF profiles.

## Error Message Policy

Public errors should be short and generic. They must not include keys,
passwords, plaintext, AAD, OpenSSL error queue details, or filesystem content.

Expected public categories:

- invalid caller input: `invalid_input`
- malformed text encoding: `invalid_encoding`
- malformed packet or file structure: `invalid_packet`
- failed authentication: `authentication_failed`
- OpenSSL, filesystem, or other backend failure: `backend_failure`

Authentication failures should not reveal whether the key, password, AAD,
ciphertext, header, or tag was wrong.

## Known Limitations

- No portable secure erasure guarantee.
- No protection for caller-owned secrets after return.
- No protection against process memory inspection.
- No protection against compromised OpenSSL providers.
- No rollback guarantee for stream outputs.
- No release-artifact signing or provenance beyond checksums yet.
