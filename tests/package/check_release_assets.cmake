if(DEFINED SECUREKIT_PACKAGE_ARTIFACT_DIR AND NOT SECUREKIT_PACKAGE_ARTIFACT_DIR STREQUAL "")
  set(_securekit_artifact_dir "${SECUREKIT_PACKAGE_ARTIFACT_DIR}")
  get_filename_component(_securekit_package_check_root "${_securekit_artifact_dir}" DIRECTORY)
elseif(DEFINED SECUREKIT_PACKAGE_CHECK_ROOT AND NOT SECUREKIT_PACKAGE_CHECK_ROOT STREQUAL "")
  set(_securekit_package_check_root "${SECUREKIT_PACKAGE_CHECK_ROOT}")
  set(_securekit_artifact_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/artifacts")
else()
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT or SECUREKIT_PACKAGE_ARTIFACT_DIR is required")
endif()

if(NOT IS_ABSOLUTE "${_securekit_package_check_root}")
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT must be absolute")
endif()
set(_securekit_package_check_root_name_path "${_securekit_package_check_root}")
cmake_path(NORMAL_PATH _securekit_package_check_root_name_path)
cmake_path(GET _securekit_package_check_root_name_path FILENAME _securekit_package_check_root_name)
if(NOT _securekit_package_check_root_name STREQUAL "package-check")
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT must end in package-check")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_NAME OR SECUREKIT_PROJECT_NAME STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_NAME is required")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_VERSION OR SECUREKIT_PROJECT_VERSION STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_VERSION is required")
endif()

if(NOT IS_DIRECTORY "${_securekit_artifact_dir}")
  message(FATAL_ERROR "Package artifact directory not found: ${_securekit_artifact_dir}")
endif()

set(_securekit_release_asset_dir "${_securekit_package_check_root}/release-assets")
set(_securekit_artifact_prefix "${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-")
set(_securekit_source_prefix "${_securekit_artifact_prefix}source.")

file(GLOB _securekit_artifacts LIST_DIRECTORIES FALSE
  "${_securekit_artifact_dir}/*")
list(SORT _securekit_artifacts)
list(LENGTH _securekit_artifacts _securekit_artifact_count)
if(_securekit_artifact_count EQUAL 0)
  message(FATAL_ERROR "No package artifacts found under ${_securekit_artifact_dir}")
endif()

set(_securekit_source_artifacts)
set(_securekit_binary_artifacts)

foreach(_securekit_artifact IN LISTS _securekit_artifacts)
  get_filename_component(_securekit_artifact_name "${_securekit_artifact}" NAME)

  string(FIND
    "${_securekit_artifact_name}"
    "${_securekit_artifact_prefix}"
    _securekit_artifact_prefix_at)
  if(NOT _securekit_artifact_prefix_at EQUAL 0)
    message(FATAL_ERROR "Package artifact version mismatch: ${_securekit_artifact_name}")
  endif()

  if(NOT _securekit_artifact_name MATCHES "(\\.zip|\\.tar\\.gz)$")
    message(FATAL_ERROR "Package artifact extension mismatch: ${_securekit_artifact_name}")
  endif()

  string(FIND
    "${_securekit_artifact_name}"
    "${_securekit_source_prefix}"
    _securekit_source_prefix_at)
  string(FIND
    "${_securekit_artifact_name}"
    "-source."
    _securekit_source_marker_at)

  if(_securekit_source_prefix_at EQUAL 0)
    list(APPEND _securekit_source_artifacts "${_securekit_artifact}")
  elseif(NOT _securekit_source_marker_at EQUAL -1)
    message(FATAL_ERROR "Package source artifact name mismatch: ${_securekit_artifact_name}")
  else()
    list(APPEND _securekit_binary_artifacts "${_securekit_artifact}")
  endif()
endforeach()

list(LENGTH _securekit_binary_artifacts _securekit_binary_artifact_count)
if(_securekit_binary_artifact_count EQUAL 0)
  message(FATAL_ERROR "No binary package artifacts found for ${SECUREKIT_PROJECT_VERSION}")
endif()

list(LENGTH _securekit_source_artifacts _securekit_source_artifact_count)
if(_securekit_source_artifact_count EQUAL 0)
  message(FATAL_ERROR "No source package artifacts found for ${SECUREKIT_PROJECT_VERSION}")
endif()

file(REMOVE_RECURSE "${_securekit_release_asset_dir}")
file(MAKE_DIRECTORY "${_securekit_release_asset_dir}")

set(_securekit_staged_names)

foreach(_securekit_source_artifact IN LISTS _securekit_source_artifacts)
  get_filename_component(_securekit_source_name "${_securekit_source_artifact}" NAME)
  list(FIND _securekit_staged_names "${_securekit_source_name}" _securekit_existing_stage_name_at)
  if(NOT _securekit_existing_stage_name_at EQUAL -1)
    message(FATAL_ERROR "Duplicate staged release asset name: ${_securekit_source_name}")
  endif()
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "${_securekit_source_artifact}"
      "${_securekit_release_asset_dir}/${_securekit_source_name}"
    RESULT_VARIABLE _securekit_stage_source_result)
  if(NOT _securekit_stage_source_result EQUAL 0)
    message(FATAL_ERROR "Failed to stage source release asset: ${_securekit_source_name}")
  endif()
  list(APPEND _securekit_staged_names "${_securekit_source_name}")
endforeach()

foreach(_securekit_binary_artifact IN LISTS _securekit_binary_artifacts)
  get_filename_component(_securekit_binary_name "${_securekit_binary_artifact}" NAME)
  set(_securekit_staged_name "local-package-${_securekit_binary_name}")
  list(FIND _securekit_staged_names "${_securekit_staged_name}" _securekit_existing_stage_name_at)
  if(NOT _securekit_existing_stage_name_at EQUAL -1)
    message(FATAL_ERROR "Duplicate staged release asset name: ${_securekit_staged_name}")
  endif()
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "${_securekit_binary_artifact}"
      "${_securekit_release_asset_dir}/${_securekit_staged_name}"
    RESULT_VARIABLE _securekit_stage_binary_result)
  if(NOT _securekit_stage_binary_result EQUAL 0)
    message(FATAL_ERROR "Failed to stage binary release asset: ${_securekit_staged_name}")
  endif()
  list(APPEND _securekit_staged_names "${_securekit_staged_name}")
endforeach()

list(SORT _securekit_staged_names)
set(_securekit_checksum_file "${_securekit_release_asset_dir}/SHA256SUMS.txt")
file(WRITE "${_securekit_checksum_file}" "")

foreach(_securekit_staged_name IN LISTS _securekit_staged_names)
  file(SHA256
    "${_securekit_release_asset_dir}/${_securekit_staged_name}"
    _securekit_staged_sha256)
  file(APPEND
    "${_securekit_checksum_file}"
    "${_securekit_staged_sha256}  ${_securekit_staged_name}\n")
endforeach()

file(STRINGS "${_securekit_checksum_file}" _securekit_checksum_lines)
list(LENGTH _securekit_staged_names _securekit_staged_count)
list(LENGTH _securekit_checksum_lines _securekit_checksum_line_count)
if(NOT _securekit_checksum_line_count EQUAL _securekit_staged_count)
  message(FATAL_ERROR "SHA256SUMS.txt line count does not match staged release asset count")
endif()

foreach(_securekit_checksum_line IN LISTS _securekit_checksum_lines)
  if(NOT _securekit_checksum_line MATCHES "^([0-9a-f]+)  (.+\\.(zip|tar\\.gz))$")
    message(FATAL_ERROR "SHA256SUMS.txt line format mismatch: ${_securekit_checksum_line}")
  endif()
  string(LENGTH "${CMAKE_MATCH_1}" _securekit_checksum_hash_length)
  if(NOT _securekit_checksum_hash_length EQUAL 64)
    message(FATAL_ERROR "SHA256SUMS.txt hash length mismatch: ${_securekit_checksum_line}")
  endif()
  if(_securekit_checksum_line MATCHES "  SHA256SUMS\\.txt$")
    message(FATAL_ERROR "SHA256SUMS.txt must not include itself")
  endif()
endforeach()
