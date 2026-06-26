# SecureKit Format Specification

This document is the compatibility reference for SecureKit serialized data.
Implementation changes that alter bytes on disk or on the wire must update this
file and the compatibility fixtures in the same change.

## Scope

The current v1 formats are:

- `SKT1`: one AES-256-GCM packet.
- `SKF1`: chunked file encryption with a caller-provided 32-byte key.
- `SKP1`: chunked file encryption with a password-derived key.

SecureKit does not expose caller-selected algorithms, caller-selected nonces, or
format negotiation in v1.

## Common Definitions

- Byte order: all multi-byte integers are unsigned big-endian.
- AES key size: 32 bytes.
- AES-GCM nonce size: 12 bytes.
- AES-GCM authentication tag size: 16 bytes.
- File chunk size: 1 MiB (`1048576` bytes).
- Cipher ID `0x01`: AES-256-GCM.
- KDF ID `0x01`: OpenSSL scrypt for `SKP1`.
- Format version `0x01`: first stable layout for each magic value.

Caller-provided AAD is authenticated but never stored in serialized data.
Decryption or opening requires the exact same AAD bytes supplied during
encryption or sealing.

## SKT1 Packet Format

`SKT1` is produced by `securekit::encrypt`, `securekit::wrap_key`, and
`packet_encryptor`.

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Magic | `SKT1` |
| 4 | 1 | Version | `0x01` |
| 5 | 12 | Nonce | Random per packet |
| 17 | N | Ciphertext | AES-256-GCM ciphertext |
| 17 + N | 16 | Tag | AES-GCM authentication tag |

The authenticated data is:

1. the 5-byte `SKT1` header,
2. caller-provided AAD.

The minimum serialized packet size is 33 bytes: 5-byte header, 12-byte nonce,
empty ciphertext, and 16-byte tag.

Malformed magic, unsupported version, missing tag, wrong key, wrong AAD, or tag
failure causes decryption to fail. Authentication failures use the generic AEAD
authentication error and do not identify which authenticated input was wrong.

## SKF1 Raw-Key File Format

`SKF1` is produced by `securekit::seal_file` with a caller-provided 32-byte key.
The caller key is not used directly for chunks. SecureKit derives a per-file key
with HKDF-SHA-256 and a random file salt.

### Header

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Magic | `SKF1` |
| 4 | 1 | Version | `0x01` |
| 5 | 1 | Cipher/KDF | `0x01` for AES-256-GCM with HKDF-SHA-256 |
| 6 | 4 | Chunk size | `0x00100000` |
| 10 | 32 | Salt | Random per file |
| 42 | 8 | Nonce prefix | Random per file |

### Chunk Record

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Chunk index | Starts at 0 |
| 4 | 4 | Plaintext size | `0` to `1048576` |
| 8 | 1 | Final flag | `0x00` non-final, `0x01` final |
| 9 | N | Ciphertext | Same size as plaintext |
| 9 + N | 16 | Tag | AES-GCM authentication tag |

Each chunk nonce is:

```text
nonce = header.nonce_prefix || chunk_index_be32
```

Each chunk authenticates:

1. the complete serialized file header,
2. the 9-byte chunk record header,
3. caller-provided AAD.

Non-final chunks must have plaintext size exactly 1 MiB. The final chunk may
have plaintext size from 0 to 1 MiB. Empty input is encoded as one final chunk
with plaintext size 0.

Readers reject malformed headers, unsupported versions, unsupported chunk
sizes, truncated records, non-monotonic chunk indexes, non-final short chunks,
chunks after the final chunk, missing final chunks, appended data, wrong keys,
wrong AAD, and tag failures.

The maximum chunk index is `0xffffffff`, so the maximum represented plaintext is
4 PiB with 1 MiB chunks.

## SKP1 Password File Format

`SKP1` is produced by `securekit::seal_file_with_password`. It uses the same
chunk record layout as `SKF1`, but derives the per-file key directly from
caller-provided password bytes with OpenSSL scrypt.

### Header

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Magic | `SKP1` |
| 4 | 1 | Version | `0x01` |
| 5 | 1 | Cipher | `0x01` for AES-256-GCM |
| 6 | 1 | KDF | `0x01` for scrypt |
| 7 | 1 | Flags | `0x00` |
| 8 | 4 | Chunk size | `0x00100000` |
| 12 | 32 | Scrypt salt | Random per file |
| 44 | 8 | Nonce prefix | Random per file |
| 52 | 4 | Scrypt N | `32768` |
| 56 | 4 | Scrypt r | `8` |
| 60 | 4 | Scrypt p | `1` |

Fixed scrypt parameters:

- `N = 32768`
- `r = 8`
- `p = 1`
- `maxmem = 64 MiB`
- output size: 32 bytes

Password bytes are accepted exactly as supplied by the caller. SecureKit does
not trim, normalize, encode, prompt for, or store passwords. Empty password
inputs are rejected.

Each chunk nonce and each chunk AAD are constructed the same way as `SKF1`,
using the complete serialized `SKP1` header.

Readers reject malformed headers, unsupported versions, unsupported cipher IDs,
unsupported KDF IDs, unsupported flags, unsupported scrypt parameters,
truncated records, non-monotonic chunk indexes, non-final short chunks, chunks
after the final chunk, missing final chunks, appended data, wrong passwords,
wrong AAD, and tag failures. Wrong passwords, wrong AAD, and tag failures use
the same generic file authentication error.

## Compatibility Rules

- Existing v1 fixture names must remain present unless an intentional format
  change replaces them.
- Any intentional byte-format change must update this document, fixtures, and
  tests in the same change.
- New KDFs or ciphers require the downgrade, bounds, and fixture gates in
  `docs/KDF_AGILITY.md` before implementation.
- Existing `SKT1`, `SKF1`, and `SKP1` behavior must not silently change under
  the same magic/version pair.

## Security Notes

`packet_decryptor::update()` can return plaintext before the packet tag is
verified. That plaintext is unverified. Callers must not release, persist, or
trust it until `finalize(tag)` succeeds.

File path open APIs commit output only after all chunk authentication succeeds.
Stream open APIs write to caller-owned streams and cannot roll back bytes already
written by the caller's stream.

See `docs/SECURITY_MODEL.md` for threat model and operational limits.
