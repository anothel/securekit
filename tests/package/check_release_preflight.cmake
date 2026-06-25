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
set(_securekit_format "${SECUREKIT_SOURCE_DIR}/docs/FORMAT.md")
set(_securekit_security_model "${SECUREKIT_SOURCE_DIR}/docs/SECURITY_MODEL.md")
set(_securekit_kdf_agility "${SECUREKIT_SOURCE_DIR}/docs/KDF_AGILITY.md")
set(_securekit_release_checklist "${SECUREKIT_SOURCE_DIR}/docs/RELEASE_CHECKLIST.md")
set(_securekit_roadmap "${SECUREKIT_SOURCE_DIR}/docs/ROADMAP.md")

foreach(_securekit_required_file IN ITEMS
    "${_securekit_cmakelists}"
    "${_securekit_readme}"
    "${_securekit_security}"
    "${_securekit_format}"
    "${_securekit_security_model}"
    "${_securekit_kdf_agility}"
    "${_securekit_release_checklist}"
    "${_securekit_roadmap}")
  if(NOT EXISTS "${_securekit_required_file}")
    message(FATAL_ERROR "Release preflight file not found: ${_securekit_required_file}")
  endif()
endforeach()

file(READ "${_securekit_cmakelists}" _securekit_cmakelists_text)
file(READ "${_securekit_readme}" _securekit_readme_text)
file(READ "${_securekit_security}" _securekit_security_text)
file(READ "${_securekit_format}" _securekit_format_text)
file(READ "${_securekit_security_model}" _securekit_security_model_text)
file(READ "${_securekit_kdf_agility}" _securekit_kdf_agility_text)
file(READ "${_securekit_release_checklist}" _securekit_release_checklist_text)
file(READ "${_securekit_roadmap}" _securekit_roadmap_text)

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
  "roadmap release version"
  "${_securekit_roadmap_text}"
  "project(... VERSION ${SECUREKIT_PROJECT_VERSION})")
_securekit_require_text(
  "roadmap release tag"
  "${_securekit_roadmap_text}"
  "${_securekit_expected_tag}")
_securekit_require_text(
  "roadmap local target release-preflight"
  "${_securekit_roadmap_text}"
  "--target release-preflight")

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

_securekit_require_text(
  "README local target release-preflight"
  "${_securekit_readme_text}"
  "--target release-preflight")
_securekit_require_text(
  "release checklist local target release-preflight"
  "${_securekit_release_checklist_text}"
  "--target release-preflight")

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
  "No release-artifact signing or provenance beyond checksums yet")

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

list(LENGTH _securekit_binary_artifacts _securekit_binary_artifact_count)
if(_securekit_binary_artifact_count EQUAL 0)
  message(FATAL_ERROR "No binary package artifacts found for ${SECUREKIT_PROJECT_VERSION}")
endif()

list(LENGTH _securekit_source_artifacts _securekit_source_artifact_count)
if(_securekit_source_artifact_count EQUAL 0)
  message(FATAL_ERROR "No source package artifacts found for ${SECUREKIT_PROJECT_VERSION}")
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
  if(NOT _securekit_checksum_line MATCHES "^([0-9a-f]+)  (.+\\.(zip|tar\\.gz))$")
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
