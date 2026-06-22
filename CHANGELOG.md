# Changelog

All notable SecureKit changes should be recorded here before a release tag is
created.

This project uses semantic versioning for release tags and CMake package
versions.

## Unreleased

### Added

- Security policy for private vulnerability reporting and disclosure scope.
- Format specification for `SKT1`, `SKF1`, and `SKP1`.
- Security model covering goals, non-goals, authentication rules, file-output
  safety, password handling, memory limits, and error policy.

### Changed

- Strengthened streaming decryptor documentation: plaintext returned by
  `packet_decryptor::update()` is explicitly unverified until `finalize()`
  succeeds.

## 0.1.0

- Initial planned release version.
