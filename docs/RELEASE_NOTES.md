# SecureKit Release Notes

## v0.2.1

### Added

- Added release archive consumption docs and checked C++ example coverage.
- Added release SPDX SBOM, GitHub provenance attestation, and package recipe
  draft checks for Homebrew, Conan, and vcpkg.

### Hardened

- Expanded `release-preflight` to cover package artifacts, source archives,
  `SHA256SUMS.txt`, release notes, SBOM, provenance, examples, benchmarks, and
  dogfood checks.
- Expanded negative compatibility coverage for `SKT1`, `SKF1`, and `SKP1`.
- Documented public API, OpenSSL provider/FIPS, KDF agility, and internal split
  policies without expanding public API or wire formats.

### Compatibility

- No intentional `SKT1`, `SKF1`, or `SKP1` format change.
- No breaking C++ API change.

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
