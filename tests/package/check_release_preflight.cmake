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
set(_securekit_release_checklist "${SECUREKIT_SOURCE_DIR}/docs/RELEASE_CHECKLIST.md")

foreach(_securekit_required_file IN ITEMS
    "${_securekit_cmakelists}"
    "${_securekit_readme}"
    "${_securekit_release_checklist}")
  if(NOT EXISTS "${_securekit_required_file}")
    message(FATAL_ERROR "Release preflight file not found: ${_securekit_required_file}")
  endif()
endforeach()

file(READ "${_securekit_cmakelists}" _securekit_cmakelists_text)
file(READ "${_securekit_readme}" _securekit_readme_text)
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
