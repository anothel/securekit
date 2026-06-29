# SecureKit Release Notes

## v0.2.0

### Added

- Added `securekit verify-file` and `securekit verify-file-password` to
  authenticate sealed files without writing plaintext output.
- Added the checked basic C++ example and release archive consumption recipe.
- Added release SPDX SBOM generation and GitHub provenance attestation checks
  for release assets.
- Added Homebrew, Conan, and vcpkg recipe draft generation from checked release
  archive checksums.

### Hardened

- Expanded negative compatibility coverage for `SKT1`, `SKF1`, and `SKP1`
  structural reject rules.
- Strengthened `release-preflight` checks for documented CLI, format,
  packaging, release asset, SBOM, and provenance claims.

### Compatibility

- No intentional `SKT1`, `SKF1`, or `SKP1` format change.
- No breaking C++ API change.
