# SecureKit Compatibility Fixtures

These files pin v1 wire-format compatibility for SecureKit.

Files are lowercase hex encodings of serialized packets/files:

- `skt1-aes256-gcm-aad.hex`: `SKT1`, key `00..00`, nonce `00..00`, plaintext `hello securekit`, AAD `record:v1`.
- `skt1-aes256-gcm-empty.hex`: `SKT1`, key `00..00`, nonce `00..00`, empty plaintext, empty AAD.
- `skt1-aes256-gcm-binary-aad.hex`: `SKT1`, key `00..1f`, nonce `f0..fb`, binary plaintext, AAD `aad:extra`.
- `skt1-key-wrap.hex`: `SKT1` packet wrapping key seed `0x10` with wrapping key seed `0x40`, AAD `key-id:primary`.
- `skf1-known-file.hex`: `SKF1` file containing `known SKF1 vector`, key `00..00`, AAD `fixture:aad`.
- `skp1-known-password-file.hex`: `SKP1` file containing `known SKP1 vector`, password `known SKP1 password`, AAD `fixture:aad`.
