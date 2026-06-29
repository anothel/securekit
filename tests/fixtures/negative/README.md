# Negative Compatibility Fixtures

These fixtures are malformed by design. Tests use them to pin specific
`docs/FORMAT.md` rejection rules.

- `skt1-missing-tag-format-minimum-size.hex`: protects `SKT1` minimum serialized
  packet size and missing tag rejection.
- `skt1-bad-magic.hex`: protects `SKT1` malformed magic rejection.
- `skt1-unsupported-version.hex`: protects `SKT1` unsupported version
  rejection.
- `skf1-non-final-short-chunk.hex`: protects `SKF1` non-final chunks must have
  plaintext size exactly 1 MiB.
- `skf1-bad-magic.hex`: protects `SKF1` malformed magic rejection.
- `skf1-unsupported-version.hex`: protects `SKF1` unsupported version
  rejection.
- `skf1-unsupported-algorithm.hex`: protects the `SKF1` Cipher/KDF header byte
  value.
- `skf1-unsupported-chunk-size.hex`: protects `SKF1` unsupported chunk size
  rejection.
- `skf1-truncated-record.hex`: protects `SKF1` truncated record rejection.
- `skf1-record-oversized-plaintext.hex`: protects the `SKF1` chunk record
  plaintext size range.
- `skf1-record-unsupported-final-flag.hex`: protects the `SKF1` chunk record
  final flag value set.
- `skf1-missing-final-chunk.hex`: protects `SKF1` readers reject files without
  a final chunk.
- `skf1-chunk-after-final.hex`: protects `SKF1` readers reject appended data
  after a final chunk.
- `skf1-non-monotonic-index.hex`: protects `SKF1` readers reject chunk indexes
  that do not start at 0 and increment by 1.
- `skp1-unsupported-flags.hex`: protects `SKP1` readers reject unsupported
  flags.
- `skp1-bad-magic.hex`: protects `SKP1` malformed magic rejection.
- `skp1-unsupported-version.hex`: protects `SKP1` unsupported version
  rejection.
- `skp1-unsupported-cipher.hex`: protects `SKP1` unsupported cipher ID
  rejection.
- `skp1-unsupported-kdf.hex`: protects `SKP1` unsupported KDF ID rejection.
- `skp1-unsupported-chunk-size.hex`: protects `SKP1` unsupported chunk size
  rejection.
- `skp1-unsupported-scrypt-params.hex`: protects `SKP1` unsupported scrypt
  parameter rejection.
- `skp1-unsupported-scrypt-n.hex`: protects `SKP1` unsupported scrypt `N`
  rejection.
- `skp1-unsupported-scrypt-r.hex`: protects `SKP1` unsupported scrypt `r`
  rejection.
- `skp1-unsupported-scrypt-p.hex`: protects `SKP1` unsupported scrypt `p`
  rejection.
- `skp1-truncated-record.hex`: protects `SKP1` truncated record rejection.
- `skp1-record-oversized-plaintext.hex`: protects the `SKP1` chunk record
  plaintext size range.
- `skp1-record-unsupported-final-flag.hex`: protects the `SKP1` chunk record
  final flag value set.
- `skp1-missing-final-chunk.hex`: protects `SKP1` readers reject files without
  a final chunk.
- `skp1-chunk-after-final.hex`: protects `SKP1` readers reject appended data
  after a final chunk.
- `skp1-non-monotonic-index.hex`: protects `SKP1` readers reject chunk indexes
  that do not start at 0 and increment by 1.

## Coverage Matrix

| Family | `FORMAT.md` reject rule group | Fixture coverage | Regression check |
| --- | --- | --- | --- |
| `SKT1` structural format rules | malformed magic, unsupported version, missing tag, minimum size | skt1-bad-magic.hex, skt1-unsupported-version.hex, skt1-missing-tag-format-minimum-size.hex | `Aead.RejectsNegativeCompatibilityFixtureMissingTag`, `Aead.RejectsNegativeCompatibilitySkt1HeaderRuleFixtures` |
| `SKF1` structural format rules | malformed magic, unsupported version, unsupported Cipher/KDF, unsupported chunk size, truncated records, oversized plaintext size, unsupported final flag, non-final short chunk, missing final chunk, chunk after final, non-monotonic or duplicate index | skf1-bad-magic.hex, skf1-unsupported-version.hex, skf1-unsupported-algorithm.hex, skf1-unsupported-chunk-size.hex, skf1-truncated-record.hex, skf1-record-oversized-plaintext.hex, skf1-record-unsupported-final-flag.hex, skf1-non-final-short-chunk.hex, skf1-missing-final-chunk.hex, skf1-chunk-after-final.hex, skf1-non-monotonic-index.hex; duplicate index is generated in test to avoid a 1 MiB fixture | `File.RejectsNegativeCompatibilityFixtureNonFinalShortChunk`, `File.RejectsNegativeCompatibilitySkf1FormatRuleFixtures`, `File.DetectsTruncationAppendAndReorder` |
| `SKP1` structural format rules | malformed magic, unsupported version, unsupported cipher, unsupported KDF, unsupported flags, unsupported chunk size, unsupported scrypt parameters, truncated records, oversized plaintext size, unsupported final flag, missing final chunk, chunk after final, non-monotonic or duplicate index | skp1-bad-magic.hex, skp1-unsupported-version.hex, skp1-unsupported-cipher.hex, skp1-unsupported-kdf.hex, skp1-unsupported-flags.hex, skp1-unsupported-chunk-size.hex, skp1-unsupported-scrypt-params.hex, skp1-unsupported-scrypt-n.hex, skp1-unsupported-scrypt-r.hex, skp1-unsupported-scrypt-p.hex, skp1-truncated-record.hex, skp1-record-oversized-plaintext.hex, skp1-record-unsupported-final-flag.hex, skp1-missing-final-chunk.hex, skp1-chunk-after-final.hex, skp1-non-monotonic-index.hex; duplicate index is generated in test to avoid a 1 MiB fixture | `File.PasswordRejectsNegativeCompatibilityFixtureUnsupportedFlags`, `File.PasswordRejectsNegativeCompatibilitySkp1FormatRuleFixtures`, `File.PasswordDetectsTruncationAppendAndReorder` |
| Generic authentication failures | wrong key, wrong password, wrong AAD, modified non-structural authenticated bytes, modified ciphertext, nonce, or tag | positive fixtures modified inside tests | `Aead.AuthenticationFailuresUseGenericMessage`, `Aead.DetectsPacketMutation`, `Aead.RejectsWrongKey`, `KeyWrap.RejectsWrongWrappingKey`, `KeyWrap.RejectsWrongAad`, `PacketStream.RejectsWrongKeyAndAadAtFinalize`, `File.AuthenticationFailuresUseGenericMessage`, `File.PasswordRejectsWrongPasswordAndAad` |
| Path output safety | no overwrite, temporary output cleanup after authentication failure | positive fixtures modified inside tests | `File.RejectsExistingOutput`, `File.RemovesTemporaryOutputAfterOpenFailure`, `File.RemovesPartialTemporaryOutputAfterLateOpenFailure` |
| Stream rollback limits | caller-owned stream output cannot be fully rolled back after accepted writes | positive fixtures modified inside tests | `File.StreamRejectsTrailingDataBeforeWritingFinalPlaintext`, `File.PasswordStreamRejectsTrailingDataBeforeWritingFinalPlaintext` |
| Future streaming formats | plaintext-before-auth behavior, output ownership, final-auth failure effects | add negative fixtures or generated regression cases with the format change | required before any new streaming magic or version is accepted |
