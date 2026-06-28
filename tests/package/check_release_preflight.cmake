if(NOT DEFINED SECUREKIT_SOURCE_DIR)
  message(FATAL_ERROR "SECUREKIT_SOURCE_DIR is required")
endif()

if(NOT DEFINED SECUREKIT_PACKAGE_CHECK_ROOT)
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT is required")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_NAME OR SECUREKIT_PROJECT_NAME STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_NAME is required")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_VERSION OR SECUREKIT_PROJECT_VERSION STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_VERSION is required")
endif()

if(NOT SECUREKIT_PROJECT_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "Project version is not SemVer x.y.z: ${SECUREKIT_PROJECT_VERSION}")
endif()

set(_securekit_expected_tag "v${SECUREKIT_PROJECT_VERSION}")
if(DEFINED SECUREKIT_RELEASE_TAG AND NOT SECUREKIT_RELEASE_TAG STREQUAL "")
  if(NOT SECUREKIT_RELEASE_TAG STREQUAL "${_securekit_expected_tag}")
    message(FATAL_ERROR "Release tag ${SECUREKIT_RELEASE_TAG} does not match project version ${SECUREKIT_PROJECT_VERSION}")
  endif()
endif()

set(_securekit_cmakelists "${SECUREKIT_SOURCE_DIR}/CMakeLists.txt")
set(_securekit_readme "${SECUREKIT_SOURCE_DIR}/README.md")
set(_securekit_security "${SECUREKIT_SOURCE_DIR}/SECURITY.md")
set(_securekit_contributing "${SECUREKIT_SOURCE_DIR}/CONTRIBUTING.md")
set(_securekit_format "${SECUREKIT_SOURCE_DIR}/docs/FORMAT.md")
set(_securekit_security_model "${SECUREKIT_SOURCE_DIR}/docs/SECURITY_MODEL.md")
set(_securekit_kdf_agility "${SECUREKIT_SOURCE_DIR}/docs/KDF_AGILITY.md")
set(_securekit_fuzzing "${SECUREKIT_SOURCE_DIR}/docs/FUZZING.md")
set(_securekit_coverage "${SECUREKIT_SOURCE_DIR}/docs/COVERAGE.md")
set(_securekit_dependency_policy "${SECUREKIT_SOURCE_DIR}/docs/DEPENDENCY_POLICY.md")
set(_securekit_verify_release "${SECUREKIT_SOURCE_DIR}/docs/VERIFY_RELEASE.md")
set(_securekit_dogfooding "${SECUREKIT_SOURCE_DIR}/docs/DOGFOODING.md")
set(_securekit_release_notes "${SECUREKIT_SOURCE_DIR}/docs/RELEASE_NOTES.md")
set(_securekit_release_checklist "${SECUREKIT_SOURCE_DIR}/docs/RELEASE_CHECKLIST.md")
set(_securekit_roadmap "${SECUREKIT_SOURCE_DIR}/docs/ROADMAP.md")
set(_securekit_internals "${SECUREKIT_SOURCE_DIR}/docs/INTERNALS.md")
set(_securekit_fixtures_readme "${SECUREKIT_SOURCE_DIR}/tests/fixtures/README.md")
set(_securekit_negative_fixtures_readme "${SECUREKIT_SOURCE_DIR}/tests/fixtures/negative/README.md")
set(_securekit_dependabot "${SECUREKIT_SOURCE_DIR}/.github/dependabot.yml")
set(_securekit_example_cmakelists "${SECUREKIT_SOURCE_DIR}/examples/basic/CMakeLists.txt")
set(_securekit_example_main "${SECUREKIT_SOURCE_DIR}/examples/basic/main.cpp")
set(_securekit_public_header "${SECUREKIT_SOURCE_DIR}/include/securekit/securekit.hpp")
set(_securekit_aead_header "${SECUREKIT_SOURCE_DIR}/include/securekit/aead.hpp")
set(_securekit_base64_header "${SECUREKIT_SOURCE_DIR}/include/securekit/base64.hpp")
set(_securekit_compare_header "${SECUREKIT_SOURCE_DIR}/include/securekit/compare.hpp")
set(_securekit_file_header "${SECUREKIT_SOURCE_DIR}/include/securekit/file.hpp")
set(_securekit_hash_header "${SECUREKIT_SOURCE_DIR}/include/securekit/hash.hpp")
set(_securekit_hex_header "${SECUREKIT_SOURCE_DIR}/include/securekit/hex.hpp")
set(_securekit_key_wrap_header "${SECUREKIT_SOURCE_DIR}/include/securekit/key_wrap.hpp")
set(_securekit_packet_stream_header "${SECUREKIT_SOURCE_DIR}/include/securekit/packet_stream.hpp")
set(_securekit_random_header "${SECUREKIT_SOURCE_DIR}/include/securekit/random.hpp")
set(_securekit_version_header "${SECUREKIT_SOURCE_DIR}/include/securekit/version.hpp")
set(_securekit_cli_source "${SECUREKIT_SOURCE_DIR}/src/cli/main.cpp")

foreach(_securekit_required_file IN ITEMS
    "${_securekit_cmakelists}"
    "${_securekit_readme}"
    "${_securekit_security}"
    "${_securekit_contributing}"
    "${_securekit_format}"
    "${_securekit_security_model}"
    "${_securekit_kdf_agility}"
    "${_securekit_fuzzing}"
    "${_securekit_coverage}"
    "${_securekit_dependency_policy}"
    "${_securekit_verify_release}"
    "${_securekit_dogfooding}"
    "${_securekit_release_notes}"
    "${_securekit_release_checklist}"
    "${_securekit_roadmap}"
    "${_securekit_internals}"
    "${_securekit_fixtures_readme}"
    "${_securekit_negative_fixtures_readme}"
    "${_securekit_dependabot}"
    "${_securekit_example_cmakelists}"
    "${_securekit_example_main}"
    "${_securekit_public_header}"
    "${_securekit_aead_header}"
    "${_securekit_base64_header}"
    "${_securekit_compare_header}"
    "${_securekit_file_header}"
    "${_securekit_hash_header}"
    "${_securekit_hex_header}"
    "${_securekit_key_wrap_header}"
    "${_securekit_packet_stream_header}"
    "${_securekit_random_header}"
    "${_securekit_version_header}"
    "${_securekit_cli_source}")
  if(NOT EXISTS "${_securekit_required_file}")
    message(FATAL_ERROR "Release preflight file not found: ${_securekit_required_file}")
  endif()
endforeach()

file(READ "${_securekit_cmakelists}" _securekit_cmakelists_text)
file(READ "${_securekit_readme}" _securekit_readme_text)
file(READ "${_securekit_security}" _securekit_security_text)
file(READ "${_securekit_contributing}" _securekit_contributing_text)
file(READ "${_securekit_format}" _securekit_format_text)
file(READ "${_securekit_security_model}" _securekit_security_model_text)
file(READ "${_securekit_kdf_agility}" _securekit_kdf_agility_text)
file(READ "${_securekit_fuzzing}" _securekit_fuzzing_text)
file(READ "${_securekit_coverage}" _securekit_coverage_text)
file(READ "${_securekit_dependency_policy}" _securekit_dependency_policy_text)
file(READ "${_securekit_verify_release}" _securekit_verify_release_text)
file(READ "${_securekit_dogfooding}" _securekit_dogfooding_text)
file(READ "${_securekit_release_notes}" _securekit_release_notes_text)
file(READ "${_securekit_release_checklist}" _securekit_release_checklist_text)
file(READ "${_securekit_roadmap}" _securekit_roadmap_text)
file(READ "${_securekit_internals}" _securekit_internals_text)
file(READ "${_securekit_fixtures_readme}" _securekit_fixtures_readme_text)
file(READ "${_securekit_negative_fixtures_readme}" _securekit_negative_fixtures_readme_text)
file(READ "${_securekit_dependabot}" _securekit_dependabot_text)
file(READ "${_securekit_example_cmakelists}" _securekit_example_cmakelists_text)
file(READ "${_securekit_example_main}" _securekit_example_main_text)
file(READ "${_securekit_public_header}" _securekit_public_header_text)
file(READ "${_securekit_aead_header}" _securekit_aead_header_text)
file(READ "${_securekit_base64_header}" _securekit_base64_header_text)
file(READ "${_securekit_compare_header}" _securekit_compare_header_text)
file(READ "${_securekit_file_header}" _securekit_file_header_text)
file(READ "${_securekit_hash_header}" _securekit_hash_header_text)
file(READ "${_securekit_hex_header}" _securekit_hex_header_text)
file(READ "${_securekit_key_wrap_header}" _securekit_key_wrap_header_text)
file(READ "${_securekit_packet_stream_header}" _securekit_packet_stream_header_text)
file(READ "${_securekit_random_header}" _securekit_random_header_text)
file(READ "${_securekit_version_header}" _securekit_version_header_text)
file(READ "${_securekit_cli_source}" _securekit_cli_source_text)

function(_securekit_require_text description haystack needle)
  string(FIND "${haystack}" "${needle}" _securekit_found_at)
  if(_securekit_found_at EQUAL -1)
    message(FATAL_ERROR "Release preflight missing ${description}: ${needle}")
  endif()
endfunction()

function(_securekit_require_regex description haystack pattern)
  string(REGEX MATCH "${pattern}" _securekit_match "${haystack}")
  if(_securekit_match STREQUAL "")
    message(FATAL_ERROR "Release preflight missing ${description}: ${pattern}")
  endif()
endfunction()

function(_securekit_require_terms description haystack)
  foreach(_securekit_term IN LISTS ARGN)
    _securekit_require_text(
      "${description}"
      "${haystack}"
      "${_securekit_term}")
  endforeach()
endfunction()

function(_securekit_forbid_text description haystack needle)
  string(FIND "${haystack}" "${needle}" _securekit_found_at)
  if(NOT _securekit_found_at EQUAL -1)
    message(FATAL_ERROR "Release preflight found out-of-scope ${description}: ${needle}")
  endif()
endfunction()

function(_securekit_forbid_terms description haystack)
  foreach(_securekit_term IN LISTS ARGN)
    _securekit_forbid_text(
      "${description}"
      "${haystack}"
      "${_securekit_term}")
  endforeach()
endfunction()

_securekit_require_regex(
  "CMake project version"
  "${_securekit_cmakelists_text}"
  "project\\([^\n\r]*[\n\r]+[ \t]*${SECUREKIT_PROJECT_NAME}[\n\r]+[ \t]*VERSION ${SECUREKIT_PROJECT_VERSION}")
_securekit_require_text(
  "README version example"
  "${_securekit_readme_text}"
  "project(... VERSION ${SECUREKIT_PROJECT_VERSION})")
_securekit_require_text(
  "README tag example"
  "${_securekit_readme_text}"
  "${_securekit_expected_tag}")
_securekit_require_text(
  "release checklist tag command"
  "${_securekit_release_checklist_text}"
  "git tag ${_securekit_expected_tag}")
_securekit_require_text(
  "release checklist push command"
  "${_securekit_release_checklist_text}"
  "git push origin ${_securekit_expected_tag}")
_securekit_require_text(
  "roadmap local target release-preflight"
  "${_securekit_roadmap_text}"
  "--target release-preflight")

_securekit_require_terms(
  "release-preflight target dependencies"
  "${_securekit_cmakelists_text}"
  "add_custom_target(release-preflight"
  "DEPENDS"
  "check"
  "examples-check"
  "dogfood-check"
  "release-workflow-check")

_securekit_forbid_terms(
  "release-preflight nested build"
  "${_securekit_cmakelists_text}"
  "--target check"
  "--target examples-check"
  "--target package-check"
  "--target dogfood-check"
  "--target release-workflow-check")

_securekit_require_terms(
  "roadmap scope guard"
  "${_securekit_roadmap_text}"
  "## Intake Rules"
  "## Current Plan"
  "dogfood-check"
  "no repeated friction"
  "Release Trust"
  "Fix Queue"
  "Do not leave accepted fixes deferred"
  "Keep v0.x public API changes minimal"
  "Active roadmap items must name an existing SecureKit surface"
  "External audit or roadmap notes are triage input only"
  "Node.js"
  "backend middleware"
  "## Recently Finished"
  "Use Git history and `docs/RELEASE_NOTES.md` for full completed-change detail"
  "Package and release trust"
  "## Not Planned")

_securekit_require_terms(
  "roadmap repository-specific candidates"
  "${_securekit_roadmap_text}"
  "`src/file.cpp` internal split"
  "CLI split"
  "README split into `docs/CLI.md` or `docs/API.md`"
  "Package-manager recipes")

_securekit_forbid_terms(
  "roadmap analysis dump"
  "${_securekit_roadmap_text}"
  "| Audit theme | SecureKit handling | Required check |"
  "| Analysis item | Disposition | Reason / gate |"
  "securekit_analysis_2026-06-28.md"
  "securekit_full_analysis_2026-06-28.md"
  "Already resolved"
  "Already resolved, keep guarded"
  "Accepted as release-confidence work"
  "## Parked"
  "Parked"
  "parked"
  "These are blocked candidates")

_securekit_require_terms(
  "dogfooding record"
  "${_securekit_dogfooding_text}"
  "cmake --build build --config Release --target dogfood-check"
  "keygen"
  "seal-file"
  "verify-file"
  "open-file"
  "seal-file-password"
  "verify-file-password"
  "open-file-password"
  "C++ consumer"
  "no repeated friction recorded"
  "No parked item is promoted")

_securekit_require_terms(
  "contributor one-command local checks"
  "${_securekit_contributing_text}"
  "One-command local check"
  "cmake --build build --config Release --target release-preflight"
  "SECUREKIT_BUILD_TESTS=ON"
  "SECUREKIT_BUILD_CLI=ON"
  "SECUREKIT_INSTALL_CLI=ON"
  "Do not add public API"
  "regression check"
  "rollback plan.")

_securekit_require_terms(
  "README verified feature claims"
  "${_securekit_readme_text}"
  "Hex encode and decode."
  "Base64 encode and decode."
  "Base64URL encode and decode."
  "SHA-256 digest."
  "HMAC-SHA-256 digest."
  "HKDF-SHA-256 key derivation."
  "Constant-time byte comparison for equal-length secret values."
  "Cryptographically secure random bytes."
  "URL-safe random token generation."
  "AES-256-GCM packet encryption and decryption."
  "Move-only packet streaming encryptor and decryptor for `SKT1`."
  "AES-256-GCM key wrapping helpers."
  "Chunked file sealing and opening with path and stream APIs."
  "Password-based chunked file sealing and opening with `SKP1` and scrypt.")

_securekit_require_terms(
  "README release surface contract"
  "${_securekit_readme_text}"
  "Release Surface Contract"
  "Public claims are limited to the C++ APIs listed in Public API"
  "listed in CLI"
  "`SKT1`, `SKF1`, and `SKP1` formats"
  "`release-preflight` checks these docs against public"
  "CLI command usage"
  "install/export/package artifacts"
  "release assets"
  "External audits and roadmap notes are handled through this contract"
  "Items that"
  "do not map to the C++ API, CLI, `SKT1`/`SKF1`/`SKP1`, CMake package"
  "asset, or security-reporting surface"
  "triage input, not implementation scope"
  "`docs/ROADMAP.md` records the active split")

_securekit_require_terms(
  "README release notes and internal boundary docs"
  "${_securekit_readme_text}"
  "The release notes source of truth is"
  "GitHub Release notes should be"
  "edited to match it"
  "SECUREKIT_BUILD_FUZZ"
  "[docs/FUZZING.md](docs/FUZZING.md)"
  "[docs/INTERNALS.md](docs/INTERNALS.md)")

_securekit_require_terms(
  "README CLI verify entry point"
  "${_securekit_readme_text}"
  "securekit verify-file"
  "securekit verify-file-password"
  "without creating a plaintext output file"
  "write nothing to stdout on success"
  "exit code: 0 on success, 1 on failure"
  "Usage, parse, file, or decoding failures return exit code 1"
  "authentication verification")

_securekit_require_terms(
  "security model CLI verify output contract"
  "${_securekit_security_model_text}"
  "verify-file"
  "verify-file-password"
  "take no output path"
  "discard"
  "recovered plaintext"
  "exit code 0"
  "exit code 1"
  "writes no stdout for failed commands")

_securekit_require_terms(
  "README contributor entry point"
  "${_securekit_readme_text}"
  "[CONTRIBUTING.md](CONTRIBUTING.md)"
  "one-command local release check")

_securekit_require_terms(
  "README example and package-manager entry points"
  "${_securekit_readme_text}"
  "[examples/basic](examples/basic)"
  "cmake --build build --config Release --target examples-check"
  "consumer"
  "CMake project against the installed package"
  "FetchContent"
  "release source archive"
  "release archive checksum")

_securekit_require_terms(
  "positive fixture provenance"
  "${_securekit_fixtures_readme_text}"
  "## Provenance and Regeneration"
  "not external standard vectors"
  "fixed keys, nonces, passwords, plaintext, AAD"
  "Do not refresh fixture bytes"
  "FixtureInventory.*"
  "release-preflight")

_securekit_require_terms(
  "negative compatibility fixture matrix"
  "${_securekit_negative_fixtures_readme_text}"
  "## Coverage Matrix"
  "| Family | `FORMAT.md` reject rule group | Fixture coverage | Regression check |"
  "`SKT1` structural format rules"
  "malformed magic, unsupported version, missing tag, minimum size"
  "`SKF1` structural format rules"
  "unsupported Cipher/KDF"
  "non-final short chunk"
  "`SKP1` structural format rules"
  "unsupported scrypt parameters"
  "skp1-unsupported-scrypt-n.hex"
  "skp1-unsupported-scrypt-r.hex"
  "skp1-unsupported-scrypt-p.hex"
  "duplicate index is generated in test"
  "Generic authentication failures"
  "Path output safety"
  "Stream rollback limits")

_securekit_require_terms(
  "fuzz corpus policy"
  "${_securekit_fuzzing_text}"
  "## Corpus Policy"
  "Keep checked-in corpus entries small, stable, and format-focused."
  "tests/fixtures"
  "tests/fuzz/corpus"
  "Minimized crash reproducers"
  "Do not check in generated fuzz output")

_securekit_require_terms(
  "coverage reporting policy"
  "${_securekit_coverage_text}"
  "Coverage is observational"
  "not a release gate"
  "SECUREKIT_ENABLE_COVERAGE=ON"
  "coverage-report"
  "gcovr"
  "securekit.html"
  "coverage.xml")

_securekit_require_terms(
  "dependency update policy"
  "${_securekit_dependency_policy_text}"
  "GitHub Actions"
  ".github/dependabot.yml"
  "release-workflow-check"
  "GoogleTest"
  "FetchContent"
  "OpenSSL Crypto 3.0 or newer"
  "Do not vendor OpenSSL"
  "release-preflight")

_securekit_require_terms(
  "Dependabot GitHub Actions updates"
  "${_securekit_dependabot_text}"
  "package-ecosystem: github-actions"
  "interval: weekly"
  "open-pull-requests-limit: 5")

_securekit_require_terms(
  "internal boundary document"
  "${_securekit_internals_text}"
  "Current Ownership"
  "`src/file.cpp`"
  "`SKF1` and `SKP1` file format parsing/serialization"
  "path open temp-file commit"
  "password header handling"
  "Split Gates"
  "must not change public C++ APIs, CLI command shape, or `SKT1`/`SKF1`/`SKP1`"
  "`file_format_internal.*`"
  "`file_io_internal.*`"
  "`file_crypto_internal.*`"
  "`password_kdf_internal.*`"
  "Do not split it just because it is long."
  "--target release-preflight")

_securekit_require_terms(
  "CMake examples check target"
  "${_securekit_cmakelists_text}"
  "securekit_example_basic"
  "examples-check")

_securekit_require_terms(
  "basic example uses public API"
  "${_securekit_example_main_text}"
  "securekit/securekit.hpp"
  "securekit::encrypt"
  "securekit::decrypt"
  "securekit::sha256")

_securekit_require_terms(
  "basic example CMake recipe"
  "${_securekit_example_cmakelists_text}"
  "find_package(securekit CONFIG"
  "securekit::securekit")

_securekit_require_terms(
  "public aggregate header feature mapping"
  "${_securekit_public_header_text}"
  "securekit/aead.hpp"
  "securekit/base64.hpp"
  "securekit/compare.hpp"
  "securekit/file.hpp"
  "securekit/hash.hpp"
  "securekit/hex.hpp"
  "securekit/key_wrap.hpp"
  "securekit/packet_stream.hpp"
  "securekit/random.hpp"
  "securekit/version.hpp")

_securekit_require_terms(
  "public hex header API mapping"
  "${_securekit_hex_header_text}"
  "hex_encode"
  "hex_decode")
_securekit_require_terms(
  "public base64 header API mapping"
  "${_securekit_base64_header_text}"
  "base64_encode"
  "base64_decode"
  "base64url_encode"
  "base64url_decode")
_securekit_require_terms(
  "public hash header API mapping"
  "${_securekit_hash_header_text}"
  "sha256"
  "hmac_sha256"
  "hkdf_sha256"
  "output_size == 0 returns an empty vector")
_securekit_require_terms(
  "public compare header API mapping"
  "${_securekit_compare_header_text}"
  "constant_time_equal"
  "Input lengths are"
  "must not be secret")
_securekit_require_terms(
  "public random header API mapping"
  "${_securekit_random_header_text}"
  "random_bytes"
  "generate_key"
  "random_token")
_securekit_require_terms(
  "public AEAD header API mapping"
  "${_securekit_aead_header_text}"
  "encrypt"
  "decrypt"
  "Authenticates the whole SKT1 packet before returning plaintext"
  "generic authentication failure")
_securekit_require_terms(
  "public key wrap header API mapping"
  "${_securekit_key_wrap_header_text}"
  "wrap_key"
  "unwrap_key")
_securekit_require_terms(
  "public packet stream header API mapping"
  "${_securekit_packet_stream_header_text}"
  "packet_encryptor"
  "packet_decryptor")
_securekit_require_terms(
  "public file header API mapping"
  "${_securekit_file_header_text}"
  "seal_file"
  "open_file"
  "seal_file_with_password"
  "open_file_with_password"
  "refuses an existing output path"
  "temporary file"
  "Stream overload writes to caller-owned output"
  "Password bytes are used exactly as supplied"
  "no trimming, normalization")
_securekit_require_terms(
  "public version header API mapping"
  "${_securekit_version_header_text}"
  "version()"
  "version_major"
  "version_minor"
  "version_patch")

_securekit_require_terms(
  "CLI release surface mapping"
  "${_securekit_cli_source_text}"
  "securekit token <byte-size>"
  "securekit sha256 --text <text>"
  "securekit hmac-sha256 --key-hex <hex>"
  "securekit hkdf-sha256 --key-hex <hex>"
  "securekit hex-encode --text <text>"
  "securekit base64url-encode --text <text>"
  "securekit wrap-key"
  "securekit unwrap-key"
  "securekit encrypt"
  "securekit decrypt"
  "securekit seal-file"
  "securekit open-file"
  "securekit verify-file"
  "securekit seal-file-password"
  "securekit open-file-password"
  "securekit verify-file-password")

_securekit_forbid_terms(
  "README web middleware scope expansion"
  "${_securekit_readme_text}"
  "Node.js"
  "package.json"
  "TEST_SUMMARY"
  "Express"
  "Koa"
  "Fastify"
  "JWT"
  "CSRF"
  "CORS"
  "NestJS"
  "rate limiting"
  "diagnostic routes")
_securekit_forbid_terms(
  "FORMAT web middleware scope expansion"
  "${_securekit_format_text}"
  "Node.js"
  "package.json"
  "TEST_SUMMARY"
  "Express"
  "Koa"
  "Fastify"
  "JWT"
  "CSRF"
  "CORS"
  "NestJS"
  "rate limiting"
  "diagnostic routes")
_securekit_forbid_terms(
  "SECURITY_MODEL web middleware scope expansion"
  "${_securekit_security_model_text}"
  "Node.js"
  "package.json"
  "TEST_SUMMARY"
  "Express"
  "Koa"
  "Fastify"
  "JWT"
  "CSRF"
  "CORS"
  "NestJS"
  "rate limiting"
  "diagnostic routes")

foreach(_securekit_target_name IN ITEMS check package-check release-workflow-check)
  _securekit_require_text(
    "README local target ${_securekit_target_name}"
    "${_securekit_readme_text}"
    "--target ${_securekit_target_name}")
  _securekit_require_text(
    "release checklist included target ${_securekit_target_name}"
    "${_securekit_release_checklist_text}"
    "${_securekit_target_name}")
endforeach()

_securekit_require_terms(
  "CMake coverage target"
  "${_securekit_cmakelists_text}"
  "SECUREKIT_ENABLE_COVERAGE"
  "coverage-report"
  "GCOVR_EXE"
  "--html-details"
  "--xml"
  "--print-summary")
_securekit_require_terms(
  "README coverage target"
  "${_securekit_readme_text}"
  "SECUREKIT_ENABLE_COVERAGE"
  "coverage-report"
  "docs/COVERAGE.md")

_securekit_require_text(
  "README local target release-preflight"
  "${_securekit_readme_text}"
  "--target release-preflight")
_securekit_require_text(
  "release checklist local target release-preflight"
  "${_securekit_release_checklist_text}"
  "--target release-preflight")
_securekit_require_terms(
  "release provenance verification docs"
  "${_securekit_release_checklist_text}"
  "GitHub artifact attestations"
  "securekit-X.Y.Z-release.spdx.json"
  "gh attestation verify SHA256SUMS.txt --repo anothel/securekit"
  "sha256sum -c SHA256SUMS.txt"
  "docs/RELEASE_NOTES.md"
  "Do not introduce a separate `CHANGELOG.md`"
  "Release notes mention the same user-visible changes"
  "run `fuzz-smoke`"
  "extra parser smoke check")
_securekit_require_terms(
  "release verification user guide"
  "${_securekit_verify_release_text}"
  "SHA256SUMS.txt"
  "sha256sum -c SHA256SUMS.txt"
  "Get-FileHash"
  "gh attestation verify SHA256SUMS.txt --repo anothel/securekit"
  "securekit-X.Y.Z-release.spdx.json"
  "Source vs Binary Archives"
  "What This Does Not Prove")
_securekit_require_terms(
  "release checklist dependency hygiene"
  "${_securekit_release_checklist_text}"
  "docs/DEPENDENCY_POLICY.md"
  "release-workflow-check"
  "OpenSSL or GoogleTest"
  "release-preflight")
_securekit_require_terms(
  "README release provenance claim"
  "${_securekit_readme_text}"
  "artifact attestations for release assets"
  "release SPDX SBOM"
  "release provenance attestation wiring")

_securekit_require_terms(
  "v0.2.0 release notes"
  "${_securekit_release_notes_text}"
  "## v0.2.0"
  "`securekit verify-file`"
  "`securekit verify-file-password`"
  "checked basic C++ example"
  "negative compatibility coverage"
  "release SPDX SBOM"
  "GitHub provenance attestation checks"
  "No intentional `SKT1`, `SKF1`, or `SKP1` format change."
  "No breaking C++ API change.")

_securekit_require_terms(
  "SECURITY private advisory path"
  "${_securekit_security_text}"
  "https://github.com/anothel/securekit/security/advisories/new"
  "https://github.com/anothel"
  "Do not publish details")
_securekit_require_terms(
  "SECURITY vulnerability report template"
  "${_securekit_security_text}"
  "SecureKit"
  "version"
  "tag"
  "commit"
  "OS"
  "compiler"
  "CMake"
  "OpenSSL"
  "provider configuration"
  "reproducer"
  "expected behavior"
  "actual behavior"
  "affected API"
  "CLI command"
  "serialized format"
  "exploitability")
_securekit_require_terms(
  "SECURITY release note requirements"
  "${_securekit_security_text}"
  "affected surface"
  "fixed version"
  "upgrade instructions"
  "`SKT1`"
  "`SKF1`"
  "`SKP1`"
  "data needs"
  "regenerated")

_securekit_require_terms(
  "FORMAT v1 serialized format docs"
  "${_securekit_format_text}"
  "# SecureKit Format Specification"
  "`SKT1`"
  "`SKF1`"
  "`SKP1`"
  "Caller-provided AAD"
  "nonce = header.nonce_prefix || chunk_index_be32"
  "Scrypt N"
  "## Compatibility Rules"
  "## Security Notes"
  "unverified"
  "Stream open APIs")

_securekit_require_terms(
  "SECURITY_MODEL goals and non-goals"
  "${_securekit_security_model_text}"
  "# SecureKit Security Model"
  "## Threat Model"
  "## Non-Goals"
  "## AEAD Authentication Rules"
  "## File Output Safety"
  "## Password-Based Encryption"
  "## Error Message Policy"
  "## Known Limitations"
  "custom cryptographic primitives"
  "TLS or networking"
  "secure key storage"
  "guaranteed memory erasure"
  "caller-selected nonces"
  "wrong AAD"
  "`packet_decryptor`"
  "unverified plaintext"
  "temporary file"
  "`N = 32768`"
  "`maxmem = 64 MiB`"
  "invalid_input"
  "invalid_encoding"
  "invalid_packet"
  "authentication_failed"
  "backend_failure"
  "Release assets are checksummed and provenance-attested by GitHub Actions"
  "Release assets include a generated SPDX SBOM")

_securekit_require_terms(
  "KDF agility downgrade and fixture gates"
  "${_securekit_kdf_agility_text}"
  "# SecureKit KDF Agility Policy"
  "Downgrade Policy"
  "fail closed"
  "memory and time upper bounds"
  "Fixture Gate"
  "at least three"
  "compatibility vectors"
  "one old `SKP1` `KDF ID 0x01` vector"
  "one new-profile vector with non-empty plaintext and AAD"
  "one new-profile vector with binary plaintext or binary AAD")

set(_securekit_artifact_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/artifacts")
if(NOT IS_DIRECTORY "${_securekit_artifact_dir}")
  message(FATAL_ERROR "Package artifact directory not found: ${_securekit_artifact_dir}")
endif()

file(GLOB _securekit_release_artifacts LIST_DIRECTORIES FALSE
  "${_securekit_artifact_dir}/*")
list(LENGTH _securekit_release_artifacts _securekit_release_artifact_count)
if(_securekit_release_artifact_count EQUAL 0)
  message(FATAL_ERROR "No package artifacts found under ${_securekit_artifact_dir}")
endif()

file(GLOB _securekit_binary_artifacts
  "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-*")
list(FILTER _securekit_binary_artifacts EXCLUDE REGEX "-source\\.")
file(GLOB _securekit_source_artifacts
  "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-source.*")
set(_securekit_source_zip_artifact "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-source.zip")
set(_securekit_source_tgz_artifact "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-source.tar.gz")

list(LENGTH _securekit_binary_artifacts _securekit_binary_artifact_count)
if(_securekit_binary_artifact_count EQUAL 0)
  message(FATAL_ERROR "No binary package artifacts found for ${SECUREKIT_PROJECT_VERSION}")
endif()

list(LENGTH _securekit_source_artifacts _securekit_source_artifact_count)
if(_securekit_source_artifact_count EQUAL 0)
  message(FATAL_ERROR "No source package artifacts found for ${SECUREKIT_PROJECT_VERSION}")
endif()
if(NOT EXISTS "${_securekit_source_zip_artifact}")
  message(FATAL_ERROR "Source ZIP package artifact not found: ${_securekit_source_zip_artifact}")
endif()
if(NOT EXISTS "${_securekit_source_tgz_artifact}")
  message(FATAL_ERROR "Source TGZ package artifact not found: ${_securekit_source_tgz_artifact}")
endif()

foreach(_securekit_release_artifact IN LISTS _securekit_release_artifacts)
  get_filename_component(_securekit_release_artifact_name "${_securekit_release_artifact}" NAME)
  if(NOT _securekit_release_artifact_name MATCHES "\\.(zip|tar\\.gz)$")
    message(FATAL_ERROR "Package artifact extension mismatch: ${_securekit_release_artifact_name}")
  endif()
  string(FIND
    "${_securekit_release_artifact_name}"
    "${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-"
    _securekit_release_artifact_prefix_at)
  if(NOT _securekit_release_artifact_prefix_at EQUAL 0)
    message(FATAL_ERROR "Package artifact version mismatch: ${_securekit_release_artifact_name}")
  endif()
endforeach()

set(_securekit_release_asset_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/release-assets")
if(NOT IS_DIRECTORY "${_securekit_release_asset_dir}")
  message(FATAL_ERROR "Release asset staging directory not found: ${_securekit_release_asset_dir}")
endif()

set(_securekit_checksum_file "${_securekit_release_asset_dir}/SHA256SUMS.txt")
if(NOT EXISTS "${_securekit_checksum_file}")
  message(FATAL_ERROR "Release asset checksum file not found: ${_securekit_checksum_file}")
endif()

set(_securekit_sbom_name "${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-release.spdx.json")
set(_securekit_sbom_file "${_securekit_release_asset_dir}/${_securekit_sbom_name}")
if(NOT EXISTS "${_securekit_sbom_file}")
  message(FATAL_ERROR "Release SBOM not found: ${_securekit_sbom_file}")
endif()
file(READ "${_securekit_sbom_file}" _securekit_sbom_text)
_securekit_require_terms(
  "release SBOM SPDX content"
  "${_securekit_sbom_text}"
  "\"spdxVersion\": \"SPDX-2.3\""
  "\"SPDXID\": \"SPDXRef-DOCUMENT\""
  "\"name\": \"${SECUREKIT_PROJECT_NAME} ${SECUREKIT_PROJECT_VERSION} release assets\""
  "\"packages\""
  "\"checksums\""
  "\"algorithm\": \"SHA256\"")

file(GLOB _securekit_staged_release_assets LIST_DIRECTORIES FALSE
  "${_securekit_release_asset_dir}/*")
list(FILTER _securekit_staged_release_assets EXCLUDE REGEX "/SHA256SUMS\\.txt$")
list(LENGTH _securekit_staged_release_assets _securekit_staged_release_asset_count)
if(_securekit_staged_release_asset_count EQUAL 0)
  message(FATAL_ERROR "No staged release assets found under ${_securekit_release_asset_dir}")
endif()

file(STRINGS "${_securekit_checksum_file}" _securekit_checksum_lines)
list(LENGTH _securekit_checksum_lines _securekit_checksum_line_count)
if(NOT _securekit_checksum_line_count EQUAL _securekit_staged_release_asset_count)
  message(FATAL_ERROR "Release asset checksum line count does not match staged asset count")
endif()

set(_securekit_checksum_asset_names)
foreach(_securekit_checksum_line IN LISTS _securekit_checksum_lines)
  if(NOT _securekit_checksum_line MATCHES "^([0-9a-f]+)  (.+\\.(zip|tar\\.gz|spdx\\.json))$")
    message(FATAL_ERROR "Release asset checksum line format mismatch: ${_securekit_checksum_line}")
  endif()
  set(_securekit_expected_sha256 "${CMAKE_MATCH_1}")
  set(_securekit_checksum_asset_name "${CMAKE_MATCH_2}")

  string(LENGTH "${CMAKE_MATCH_1}" _securekit_checksum_hash_length)
  if(NOT _securekit_checksum_hash_length EQUAL 64)
    message(FATAL_ERROR "Release asset checksum hash length mismatch: ${_securekit_checksum_line}")
  endif()
  if(_securekit_checksum_line MATCHES "  SHA256SUMS\\.txt$")
    message(FATAL_ERROR "Release asset checksum file must not include itself")
  endif()
  string(FIND "${_securekit_checksum_asset_name}" "/" _securekit_checksum_asset_slash_at)
  string(FIND "${_securekit_checksum_asset_name}" "\\" _securekit_checksum_asset_backslash_at)
  if(NOT _securekit_checksum_asset_slash_at EQUAL -1 OR NOT _securekit_checksum_asset_backslash_at EQUAL -1)
    message(FATAL_ERROR "Release asset checksum name must be a file name: ${_securekit_checksum_asset_name}")
  endif()

  list(FIND _securekit_checksum_asset_names "${_securekit_checksum_asset_name}" _securekit_checksum_duplicate_at)
  if(NOT _securekit_checksum_duplicate_at EQUAL -1)
    message(FATAL_ERROR "Duplicate release asset checksum entry: ${_securekit_checksum_asset_name}")
  endif()
  list(APPEND _securekit_checksum_asset_names "${_securekit_checksum_asset_name}")

  set(_securekit_staged_asset "${_securekit_release_asset_dir}/${_securekit_checksum_asset_name}")
  if(NOT EXISTS "${_securekit_staged_asset}")
    message(FATAL_ERROR "Release asset checksum entry has no staged file: ${_securekit_checksum_asset_name}")
  endif()
  file(SHA256 "${_securekit_staged_asset}" _securekit_actual_sha256)
  if(NOT _securekit_actual_sha256 STREQUAL _securekit_expected_sha256)
    message(FATAL_ERROR "Release asset checksum mismatch: ${_securekit_checksum_asset_name}")
  endif()
endforeach()
