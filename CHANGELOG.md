# Changelog

All notable SecureKit changes should be recorded here before a release tag is
created.

This project uses semantic versioning for release tags and CMake package
versions.

## Unreleased

### Changed

- Added best-effort internal wiping for derived file keys and temporary decrypted key/plaintext buffers.
- Added optional Clang/libFuzzer scaffolding for strict decoders, `SKT1`, and file open paths.
- Documented KDF agility downgrade, bounds, and fixture gates before future password-file profile changes.
- Strengthened release preflight to recalculate staged release asset checksums.
- Strengthened release preflight to validate generated release notes content.
- Hardened package-check cleanup roots against accidental broad deletion.

## 0.1.0

### Added

- Initial planned release version.
- Security policy for private vulnerability reporting and disclosure scope.
- Format specification for `SKT1`, `SKF1`, and `SKP1`.
- Security model covering goals, non-goals, authentication rules, file-output
  safety, password handling, memory limits, and error policy.
- Public runtime version API for reporting the SecureKit runtime version.
- CLI build/install CMake options for configuring command-line tool builds and
  installs.

### Changed

- Strengthened streaming decryptor documentation: plaintext returned by
  `packet_decryptor::update()` is explicitly unverified until `finalize()`
  succeeds.

### Validation

- Library-only package validation and consumer coverage for installed package
  use without the CLI.
- Warnings-as-errors, sanitizer, and macOS CI/package coverage for release
  confidence.
- Release/package preflight validation for local release readiness checks.
- Local release asset staging and SHA-256 checksum validation for package-check
  artifacts.
