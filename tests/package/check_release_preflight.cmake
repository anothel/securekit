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
set(_securekit_changelog "${SECUREKIT_SOURCE_DIR}/CHANGELOG.md")
set(_securekit_release_checklist "${SECUREKIT_SOURCE_DIR}/docs/RELEASE_CHECKLIST.md")

foreach(_securekit_required_file IN ITEMS
    "${_securekit_cmakelists}"
    "${_securekit_readme}"
    "${_securekit_changelog}"
    "${_securekit_release_checklist}")
  if(NOT EXISTS "${_securekit_required_file}")
    message(FATAL_ERROR "Release preflight file not found: ${_securekit_required_file}")
  endif()
endforeach()

file(READ "${_securekit_cmakelists}" _securekit_cmakelists_text)
file(READ "${_securekit_readme}" _securekit_readme_text)
file(READ "${_securekit_changelog}" _securekit_changelog_text)
file(READ "${_securekit_release_checklist}" _securekit_release_checklist_text)

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

string(REGEX REPLACE "\\." "\\\\." _securekit_project_version_regex "${SECUREKIT_PROJECT_VERSION}")
_securekit_require_regex(
  "CHANGELOG Unreleased section"
  "${_securekit_changelog_text}"
  "(^|[\n\r]+)##[ \t]+Unreleased([ \t\r\n]|$)")
_securekit_require_regex(
  "CHANGELOG current version section"
  "${_securekit_changelog_text}"
  "(^|[\n\r]+)##[ \t]+(\\[${_securekit_project_version_regex}\\]|${_securekit_project_version_regex})([ \t\r\n]|$)")

set(_securekit_changelog_scan_text "\n${_securekit_changelog_text}")
set(_securekit_changelog_current_section "")
foreach(_securekit_current_heading IN ITEMS
    "## ${SECUREKIT_PROJECT_VERSION}"
    "## [${SECUREKIT_PROJECT_VERSION}]")
  string(FIND
    "${_securekit_changelog_scan_text}"
    "\n${_securekit_current_heading}"
    _securekit_changelog_current_heading_at)
  if(NOT _securekit_changelog_current_heading_at EQUAL -1)
    math(EXPR _securekit_changelog_current_section_start "${_securekit_changelog_current_heading_at} + 1")
    string(SUBSTRING
      "${_securekit_changelog_scan_text}"
      ${_securekit_changelog_current_section_start}
      -1
      _securekit_changelog_after_current_heading)
    string(FIND
      "${_securekit_changelog_after_current_heading}"
      "\n## "
      _securekit_changelog_next_heading_at)
    if(_securekit_changelog_next_heading_at EQUAL -1)
      set(_securekit_changelog_current_section "${_securekit_changelog_after_current_heading}")
    else()
      string(SUBSTRING
        "${_securekit_changelog_after_current_heading}"
        0
        ${_securekit_changelog_next_heading_at}
        _securekit_changelog_current_section)
    endif()
    break()
  endif()
endforeach()

if(_securekit_changelog_current_section STREQUAL "")
  message(FATAL_ERROR "Release preflight could not extract CHANGELOG current version section")
endif()

string(TOLOWER "${_securekit_changelog_current_section}" _securekit_changelog_current_section_lower)
_securekit_require_terms(
  "CHANGELOG security policy docs"
  "${_securekit_changelog_current_section_lower}"
  "security policy")
_securekit_require_terms(
  "CHANGELOG format specification docs"
  "${_securekit_changelog_current_section_lower}"
  "format specification")
_securekit_require_terms(
  "CHANGELOG security model docs"
  "${_securekit_changelog_current_section_lower}"
  "security model")
_securekit_require_terms(
  "CHANGELOG streaming decryptor warning"
  "${_securekit_changelog_current_section_lower}"
  "streaming decryptor")
_securekit_require_terms(
  "CHANGELOG unverified plaintext warning"
  "${_securekit_changelog_current_section_lower}"
  "unverified"
  "plaintext")
_securekit_require_terms(
  "CHANGELOG public runtime version API"
  "${_securekit_changelog_current_section_lower}"
  "runtime"
  "version"
  "api")
_securekit_require_terms(
  "CHANGELOG CLI build/install CMake options"
  "${_securekit_changelog_current_section_lower}"
  "cli"
  "build"
  "install"
  "cmake"
  "options")
_securekit_require_terms(
  "CHANGELOG library-only package validation"
  "${_securekit_changelog_current_section_lower}"
  "library-only package validation")
_securekit_require_terms(
  "CHANGELOG consumer coverage"
  "${_securekit_changelog_current_section_lower}"
  "consumer coverage")
_securekit_require_terms(
  "CHANGELOG warnings-as-errors coverage"
  "${_securekit_changelog_current_section_lower}"
  "warnings-as-errors")
_securekit_require_terms(
  "CHANGELOG sanitizer coverage"
  "${_securekit_changelog_current_section_lower}"
  "sanitizer")
_securekit_require_terms(
  "CHANGELOG macOS CI/package coverage"
  "${_securekit_changelog_current_section_lower}"
  "macos"
  "ci"
  "package"
  "coverage")
_securekit_require_terms(
  "CHANGELOG release/package preflight validation"
  "${_securekit_changelog_current_section_lower}"
  "release"
  "package"
  "preflight"
  "validation")

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
