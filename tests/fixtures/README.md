# SecureKit Compatibility Fixtures

These files pin v1 wire-format compatibility for SecureKit.

## Compatibility Vector Policy

Fixture inventory must include at least:

- Three `SKT1` AEAD packet fixtures named skt1-aes256-gcm-*.hex.
- Two `SKT1` key wrapping fixtures named skt1-key-wrap*.hex.
- Three `SKF1` file fixtures named skf1-*.hex.
- Two `SKP1` password file fixtures named skp1-*.hex.

Every hex fixture must be lowercase, canonical, non-empty, even-length
hex and documented exactly once in this README.

Intentional wire-format behavior changes must add or update the known-answer
fixture and README entry in the same change.

Existing baseline fixture names must remain present unless a deliberate v1
compatibility change replaces them.

Malformed compatibility fixtures live under `negative/` and must name the
specific `docs/FORMAT.md` rejection rule they protect.

## Inventory

Files are lowercase hex encodings of serialized packets/files:

- `skt1-aes256-gcm-aad.hex`: `SKT1`, key `00..00`, nonce `00..00`, plaintext `hello securekit`, AAD `record:v1`.
- `skt1-aes256-gcm-empty.hex`: `SKT1`, key `00..00`, nonce `00..00`, empty plaintext, empty AAD.
- `skt1-aes256-gcm-binary-aad.hex`: `SKT1`, key `00..1f`, nonce `f0..fb`, binary plaintext, AAD `aad:extra`.
- `skt1-key-wrap.hex`: `SKT1` packet wrapping key seed `0x10` with wrapping key seed `0x40`, AAD `key-id:primary`.
- `skt1-key-wrap-zero.hex`: `SKT1` packet wrapping an all-zero key with wrapping key seed `0x80`, empty AAD.
- `skf1-known-file.hex`: `SKF1` file containing `known SKF1 vector`, key `00..00`, AAD `fixture:aad`.
- `skf1-empty-aad.hex`: `SKF1` file containing empty plaintext, key `00..00`, AAD `fixture:empty`.
- `skf1-binary-aad.hex`: `SKF1` file containing binary plaintext, key `00..1f`, AAD `fixture:binary`.
- `skp1-known-password-file.hex`: `SKP1` file containing `known SKP1 vector`, password `known SKP1 password`, AAD `fixture:aad`.
- `skp1-binary-aad.hex`: `SKP1` file containing binary plaintext, password `known SKP1 binary password`, AAD `fixture:password:binary`.
- `negative/skt1-missing-tag-format-minimum-size.hex`: malformed `SKT1` packet shorter than the 33-byte minimum.
- `negative/skt1-bad-magic.hex`: malformed `SKT1` packet with wrong magic.
- `negative/skt1-unsupported-version.hex`: malformed `SKT1` packet with unsupported version.
- `negative/skf1-non-final-short-chunk.hex`: malformed `SKF1` file with a non-final chunk smaller than 1 MiB.
- `negative/skf1-bad-magic.hex`: malformed `SKF1` header with wrong magic.
- `negative/skf1-unsupported-version.hex`: malformed `SKF1` header with unsupported version.
- `negative/skf1-unsupported-chunk-size.hex`: malformed `SKF1` header with unsupported chunk size.
- `negative/skf1-truncated-record.hex`: malformed `SKF1` file with truncated final chunk record.
- `negative/skf1-missing-final-chunk.hex`: malformed `SKF1` file with no final chunk.
- `negative/skf1-chunk-after-final.hex`: malformed `SKF1` file with data after the final chunk.
- `negative/skf1-non-monotonic-index.hex`: malformed `SKF1` file whose first chunk index is not 0.
- `negative/skp1-unsupported-flags.hex`: malformed `SKP1` header with unsupported flags.
- `negative/skp1-bad-magic.hex`: malformed `SKP1` header with wrong magic.
- `negative/skp1-unsupported-version.hex`: malformed `SKP1` header with unsupported version.
- `negative/skp1-unsupported-cipher.hex`: malformed `SKP1` header with unsupported cipher ID.
- `negative/skp1-unsupported-kdf.hex`: malformed `SKP1` header with unsupported KDF ID.
- `negative/skp1-unsupported-scrypt-params.hex`: malformed `SKP1` header with unsupported scrypt parameters.
- `negative/skp1-truncated-record.hex`: malformed `SKP1` file with truncated final chunk record.
- `negative/skp1-missing-final-chunk.hex`: malformed `SKP1` file with no final chunk.
- `negative/skp1-chunk-after-final.hex`: malformed `SKP1` file with data after the final chunk.
- `negative/skp1-non-monotonic-index.hex`: malformed `SKP1` file whose first chunk index is not 0.
