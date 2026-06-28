# SecureKit Dependency Update Policy

SecureKit keeps the runtime dependency surface small. Release-impacting
dependency changes must pass `release-preflight`.

## GitHub Actions

GitHub Actions are tracked by `.github/dependabot.yml`. Dependabot opens weekly
updates for workflow actions. Action updates must keep `release-workflow-check`
passing because release assets, checksums, SBOM generation, and provenance
attestation are part of the release contract.

## GoogleTest

Tests first use an installed `GTest` package. If none is available, CMake
fetches GoogleTest through `FetchContent`.

When changing the fallback version:

1. Update the `GIT_TAG` in `CMakeLists.txt`.
2. Update any GoogleTest cache-key text in `.github/workflows/securekit-ci.yml`.
3. Update README dependency notes if the supported workflow changes.
4. Run `release-preflight`.

Consumers do not need GoogleTest unless they build SecureKit tests. Offline
test builds can pass `-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/path/to/googletest`.

## OpenSSL

SecureKit requires OpenSSL Crypto 3.0 or newer. CI uses system packages on
Linux, Homebrew OpenSSL 3 on macOS, and vcpkg OpenSSL on Windows.

OpenSSL version or provider policy changes must update README,
`docs/SECURITY_MODEL.md`, and release checks when behavior changes. FIPS or
provider-specific support is not implied by linking OpenSSL; it needs a
separate support policy and tests before becoming a documented guarantee.

## Vendoring

Do not vendor OpenSSL. Do not add new runtime dependencies unless they solve a
specific SecureKit API, CLI, packaging, or release problem with a runnable
regression check.
