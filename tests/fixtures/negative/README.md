# Negative Compatibility Fixtures

These fixtures are malformed by design. Tests use them to pin specific
`docs/FORMAT.md` rejection rules.

- `skt1-missing-tag-format-minimum-size.hex`: protects `SKT1` minimum serialized
  packet size and missing tag rejection.
- `skf1-non-final-short-chunk.hex`: protects `SKF1` non-final chunks must have
  plaintext size exactly 1 MiB.
- `skp1-unsupported-flags.hex`: protects `SKP1` readers reject unsupported
  flags.
