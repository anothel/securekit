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
- `skf1-unsupported-chunk-size.hex`: protects `SKF1` unsupported chunk size
  rejection.
- `skf1-truncated-record.hex`: protects `SKF1` truncated record rejection.
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
- `skp1-unsupported-scrypt-params.hex`: protects `SKP1` unsupported scrypt
  parameter rejection.
- `skp1-truncated-record.hex`: protects `SKP1` truncated record rejection.
- `skp1-missing-final-chunk.hex`: protects `SKP1` readers reject files without
  a final chunk.
- `skp1-chunk-after-final.hex`: protects `SKP1` readers reject appended data
  after a final chunk.
- `skp1-non-monotonic-index.hex`: protects `SKP1` readers reject chunk indexes
  that do not start at 0 and increment by 1.

## Coverage Matrix

- `SKT1` structural format rules: skt1-bad-magic.hex,
  skt1-unsupported-version.hex, and
  skt1-missing-tag-format-minimum-size.hex; covered by
  `Aead.RejectsNegativeCompatibilityFixtureMissingTag` and
  `Aead.RejectsNegativeCompatibilitySkt1HeaderRuleFixtures`.
- `SKF1` structural format rules: skf1-bad-magic.hex,
  skf1-unsupported-version.hex, skf1-unsupported-chunk-size.hex,
  skf1-truncated-record.hex, skf1-non-final-short-chunk.hex,
  skf1-missing-final-chunk.hex, skf1-chunk-after-final.hex, and
  skf1-non-monotonic-index.hex; covered by
  `File.RejectsNegativeCompatibilityFixtureNonFinalShortChunk` and
  `File.RejectsNegativeCompatibilitySkf1FormatRuleFixtures`.
- `SKP1` structural format rules: skp1-bad-magic.hex,
  skp1-unsupported-version.hex, skp1-unsupported-cipher.hex,
  skp1-unsupported-kdf.hex, skp1-unsupported-flags.hex,
  skp1-unsupported-scrypt-params.hex, skp1-truncated-record.hex,
  skp1-missing-final-chunk.hex, skp1-chunk-after-final.hex, and
  skp1-non-monotonic-index.hex; covered by
  `File.PasswordRejectsNegativeCompatibilityFixtureUnsupportedFlags` and
  `File.PasswordRejectsNegativeCompatibilitySkp1FormatRuleFixtures`.
- Generic authentication failures are covered by
  `File.AuthenticationFailuresUseGenericMessage` and
  `File.PasswordRejectsWrongPasswordAndAad`.
- Path output safety is covered by `File.RejectsExistingOutput`,
  `File.RemovesTemporaryOutputAfterOpenFailure`, and
  `File.RemovesPartialTemporaryOutputAfterLateOpenFailure`.
- Stream rollback limits are covered by
  `File.StreamRejectsTrailingDataBeforeWritingFinalPlaintext` and
  `File.PasswordStreamRejectsTrailingDataBeforeWritingFinalPlaintext`.
