if(NOT DEFINED SECUREKIT_SOURCE_DIR)
  message(FATAL_ERROR "SECUREKIT_SOURCE_DIR is required")
endif()

if(NOT DEFINED SECUREKIT_BUILD_DIR)
  message(FATAL_ERROR "SECUREKIT_BUILD_DIR is required")
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

if(NOT DEFINED SECUREKIT_PROJECT_VERSION_MAJOR OR SECUREKIT_PROJECT_VERSION_MAJOR STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_VERSION_MAJOR is required")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_VERSION_MINOR OR SECUREKIT_PROJECT_VERSION_MINOR STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_VERSION_MINOR is required")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_VERSION_PATCH OR SECUREKIT_PROJECT_VERSION_PATCH STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_VERSION_PATCH is required")
endif()

if(NOT DEFINED SECUREKIT_BUILD_CONFIG OR SECUREKIT_BUILD_CONFIG STREQUAL "")
  if(DEFINED SECUREKIT_DEFAULT_BUILD_TYPE AND NOT SECUREKIT_DEFAULT_BUILD_TYPE STREQUAL "")
    set(SECUREKIT_BUILD_CONFIG "${SECUREKIT_DEFAULT_BUILD_TYPE}")
  else()
    set(SECUREKIT_BUILD_CONFIG "Release")
  endif()
endif()

if(NOT IS_ABSOLUTE "${SECUREKIT_PACKAGE_CHECK_ROOT}")
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT must be absolute")
endif()
set(_securekit_package_check_root_name_path "${SECUREKIT_PACKAGE_CHECK_ROOT}")
cmake_path(NORMAL_PATH _securekit_package_check_root_name_path)
cmake_path(GET _securekit_package_check_root_name_path FILENAME _securekit_package_check_root_name)
if(NOT _securekit_package_check_root_name STREQUAL "package-check")
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT must end in package-check")
endif()

set(_securekit_install_prefix "${SECUREKIT_PACKAGE_CHECK_ROOT}/install")
set(_securekit_artifact_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/artifacts")
set(_securekit_consumer_build_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/consumer-build")
set(_securekit_source_extract_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/source-extract")
set(_securekit_source_build_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/source-build")
set(_securekit_source_install_prefix "${SECUREKIT_PACKAGE_CHECK_ROOT}/source-install")
set(_securekit_library_only_build_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/library-only-build")
set(_securekit_library_only_install_prefix "${SECUREKIT_PACKAGE_CHECK_ROOT}/library-only-install")
set(_securekit_library_only_consumer_build_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/library-only-consumer-build")

file(REMOVE_RECURSE "${_securekit_install_prefix}")
file(REMOVE_RECURSE "${_securekit_artifact_dir}")
file(REMOVE_RECURSE "${_securekit_consumer_build_dir}")
file(REMOVE_RECURSE "${_securekit_source_extract_dir}")
file(REMOVE_RECURSE "${_securekit_source_build_dir}")
file(REMOVE_RECURSE "${_securekit_source_install_prefix}")
file(REMOVE_RECURSE "${_securekit_library_only_build_dir}")
file(REMOVE_RECURSE "${_securekit_library_only_install_prefix}")
file(REMOVE_RECURSE "${_securekit_library_only_consumer_build_dir}")
file(MAKE_DIRECTORY "${SECUREKIT_PACKAGE_CHECK_ROOT}")
file(MAKE_DIRECTORY "${_securekit_artifact_dir}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${SECUREKIT_BUILD_DIR}" --config "${SECUREKIT_BUILD_CONFIG}" --prefix "${_securekit_install_prefix}"
  RESULT_VARIABLE _securekit_install_result)
if(NOT _securekit_install_result EQUAL 0)
  message(FATAL_ERROR "Package install failed")
endif()

if(WIN32)
  set(_securekit_exe_suffix ".exe")
  set(_securekit_path_separator ";")
else()
  set(_securekit_exe_suffix "")
  set(_securekit_path_separator ":")
endif()

if(NOT DEFINED SECUREKIT_CPACK_COMMAND OR SECUREKIT_CPACK_COMMAND STREQUAL "")
  get_filename_component(_securekit_cmake_dir "${CMAKE_COMMAND}" DIRECTORY)
  set(SECUREKIT_CPACK_COMMAND "${_securekit_cmake_dir}/cpack${_securekit_exe_suffix}")
endif()

function(_securekit_require_archive_member archive_path member_regex)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar tf "${archive_path}"
    RESULT_VARIABLE _securekit_tar_result
    OUTPUT_VARIABLE _securekit_tar_output
    ERROR_VARIABLE _securekit_tar_error)
  if(NOT _securekit_tar_result EQUAL 0)
    message(FATAL_ERROR "Failed to list archive ${archive_path}: ${_securekit_tar_error}")
  endif()
  if(NOT _securekit_tar_output MATCHES "${member_regex}")
    message(FATAL_ERROR "Archive ${archive_path} is missing member matching ${member_regex}")
  endif()
endfunction()

set(_securekit_cli "${_securekit_install_prefix}/bin/securekit${_securekit_exe_suffix}")
string(TOLOWER "${SECUREKIT_BUILD_CONFIG}" _securekit_build_config_lower)
set(_securekit_target_files
  "${_securekit_install_prefix}/lib/cmake/securekit/securekitTargets-${SECUREKIT_BUILD_CONFIG}.cmake"
  "${_securekit_install_prefix}/lib/cmake/securekit/securekitTargets-${_securekit_build_config_lower}.cmake")

set(_securekit_found_target_file FALSE)
foreach(_securekit_target_file IN LISTS _securekit_target_files)
  if(EXISTS "${_securekit_target_file}")
    set(_securekit_found_target_file TRUE)
    break()
  endif()
endforeach()
if(NOT _securekit_found_target_file)
  message(FATAL_ERROR "Missing package target files under ${_securekit_install_prefix}/lib/cmake/securekit")
endif()

foreach(_securekit_package_file
    "${_securekit_cli}"
    "${_securekit_install_prefix}/lib/cmake/securekit/securekitConfig.cmake"
    "${_securekit_install_prefix}/lib/cmake/securekit/securekitConfigVersion.cmake"
    "${_securekit_install_prefix}/lib/cmake/securekit/securekitTargets.cmake")
  if(NOT EXISTS "${_securekit_package_file}")
    message(FATAL_ERROR "Missing package file: ${_securekit_package_file}")
  endif()
endforeach()

file(GLOB _securekit_export_files
  "${_securekit_install_prefix}/lib/cmake/securekit/securekitTargets*.cmake")
foreach(_securekit_export_file IN LISTS _securekit_export_files)
  file(READ "${_securekit_export_file}" _securekit_export_text)
  if(_securekit_export_text MATCHES "(-Werror|/WX)")
    message(FATAL_ERROR "Warnings-as-errors leaked into exported target: ${_securekit_export_file}")
  endif()
endforeach()

set(_securekit_path "$ENV{PATH}")
if(WIN32)
  set(_securekit_runtime_paths "${_securekit_install_prefix}/bin")
  if(DEFINED SECUREKIT_OPENSSL_RUNTIME_DIR AND NOT SECUREKIT_OPENSSL_RUNTIME_DIR STREQUAL "")
    list(APPEND _securekit_runtime_paths "${SECUREKIT_OPENSSL_RUNTIME_DIR}")
  elseif(DEFINED OPENSSL_ROOT_DIR AND NOT OPENSSL_ROOT_DIR STREQUAL "")
    list(APPEND _securekit_runtime_paths "${OPENSSL_ROOT_DIR}/bin")
  endif()
  list(JOIN _securekit_runtime_paths "${_securekit_path_separator}" _securekit_runtime_path)
  set(_securekit_path "${_securekit_runtime_path}${_securekit_path_separator}$ENV{PATH}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_securekit_path}"
    "${CMAKE_COMMAND}"
    "-DSECUREKIT_CLI=${_securekit_cli}"
    "-DSECUREKIT_CLI_WORK_DIR=${SECUREKIT_PACKAGE_CHECK_ROOT}/installed-cli"
    "-DSECUREKIT_TEST_FIXTURE_DIR=${SECUREKIT_SOURCE_DIR}/tests/fixtures"
    -P "${SECUREKIT_SOURCE_DIR}/tests/package/check_installed_cli.cmake"
  RESULT_VARIABLE _securekit_cli_check_result)
if(NOT _securekit_cli_check_result EQUAL 0)
  message(FATAL_ERROR "Installed CLI check failed")
endif()

execute_process(
  COMMAND "${SECUREKIT_CPACK_COMMAND}"
    --config "${SECUREKIT_BUILD_DIR}/CPackConfig.cmake"
    -C "${SECUREKIT_BUILD_CONFIG}"
    -B "${_securekit_artifact_dir}"
  RESULT_VARIABLE _securekit_binary_cpack_result)
if(NOT _securekit_binary_cpack_result EQUAL 0)
  message(FATAL_ERROR "Binary package creation failed")
endif()

execute_process(
  COMMAND "${SECUREKIT_CPACK_COMMAND}"
    --config "${SECUREKIT_BUILD_DIR}/CPackSourceConfig.cmake"
    -B "${_securekit_artifact_dir}"
  RESULT_VARIABLE _securekit_source_cpack_result)
if(NOT _securekit_source_cpack_result EQUAL 0)
  message(FATAL_ERROR "Source package creation failed")
endif()

file(GLOB _securekit_binary_artifacts
  "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-*")
list(FILTER _securekit_binary_artifacts EXCLUDE REGEX "-source\\.")
file(GLOB _securekit_source_artifacts
  "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-source.*")

list(LENGTH _securekit_binary_artifacts _securekit_binary_artifact_count)
if(_securekit_binary_artifact_count EQUAL 0)
  message(FATAL_ERROR "No binary package artifacts found under ${_securekit_artifact_dir}")
endif()

list(LENGTH _securekit_source_artifacts _securekit_source_artifact_count)
if(_securekit_source_artifact_count EQUAL 0)
  message(FATAL_ERROR "No source package artifacts found under ${_securekit_artifact_dir}")
endif()

foreach(_securekit_binary_artifact IN LISTS _securekit_binary_artifacts)
  _securekit_require_archive_member(
    "${_securekit_binary_artifact}"
    "/bin/securekit${_securekit_exe_suffix}(\r?\n|$)")
  _securekit_require_archive_member(
    "${_securekit_binary_artifact}"
    "/include/securekit/securekit\\.hpp(\r?\n|$)")
  _securekit_require_archive_member(
    "${_securekit_binary_artifact}"
    "/lib/cmake/securekit/securekitConfig\\.cmake(\r?\n|$)")
endforeach()

foreach(_securekit_source_artifact IN LISTS _securekit_source_artifacts)
  _securekit_require_archive_member(
    "${_securekit_source_artifact}"
    "/CMakeLists\\.txt(\r?\n|$)")
  _securekit_require_archive_member(
    "${_securekit_source_artifact}"
    "/include/securekit/securekit\\.hpp(\r?\n|$)")
  _securekit_require_archive_member(
    "${_securekit_source_artifact}"
    "/tests/package/check_package\\.cmake(\r?\n|$)")
  _securekit_require_archive_member(
    "${_securekit_source_artifact}"
    "/docs/RELEASE_CHECKLIST\\.md(\r?\n|$)")
  _securekit_require_archive_member(
    "${_securekit_source_artifact}"
    "/docs/ROADMAP\\.md(\r?\n|$)")
endforeach()

set(_securekit_consumer_configure_args
  -S "${SECUREKIT_SOURCE_DIR}/tests/consumer"
  -B "${_securekit_consumer_build_dir}"
  "-DCMAKE_PREFIX_PATH=${_securekit_install_prefix}"
  "-DSECUREKIT_EXPECTED_VERSION=${SECUREKIT_PROJECT_VERSION}"
  "-DSECUREKIT_EXPECTED_VERSION_MAJOR=${SECUREKIT_PROJECT_VERSION_MAJOR}"
  "-DSECUREKIT_EXPECTED_VERSION_MINOR=${SECUREKIT_PROJECT_VERSION_MINOR}"
  "-DSECUREKIT_EXPECTED_VERSION_PATCH=${SECUREKIT_PROJECT_VERSION_PATCH}"
  "-DCMAKE_BUILD_TYPE=${SECUREKIT_BUILD_CONFIG}")

set(_securekit_source_configure_args
  -B "${_securekit_source_build_dir}"
  -DBUILD_TESTING=OFF
  -DSECUREKIT_BUILD_TESTS=OFF
  "-DSECUREKIT_WARNINGS_AS_ERRORS=${SECUREKIT_WARNINGS_AS_ERRORS}"
  "-DCMAKE_INSTALL_PREFIX=${_securekit_source_install_prefix}"
  "-DCMAKE_BUILD_TYPE=${SECUREKIT_BUILD_CONFIG}")

function(_securekit_join_cmdline output_variable)
  set(_securekit_command_line "")
  foreach(_securekit_argument IN LISTS ARGN)
    if(_securekit_argument MATCHES "\"")
      message(FATAL_ERROR "Cannot quote command argument containing double quote: ${_securekit_argument}")
    endif()
    string(APPEND _securekit_command_line " \"${_securekit_argument}\"")
  endforeach()
  set(${output_variable} "${_securekit_command_line}" PARENT_SCOPE)
endfunction()

function(_securekit_run_consumer_command result_variable script_stem)
  if(_securekit_use_msvc_env)
    _securekit_join_cmdline(_securekit_command_line ${ARGN})
    set(_securekit_script "${SECUREKIT_PACKAGE_CHECK_ROOT}/${script_stem}.cmd")
    file(WRITE "${_securekit_script}"
      "@echo off\r\n"
      "call \"${_securekit_vsdevcmd}\" -arch=x64 -host_arch=x64 >nul\r\n"
      "${_securekit_command_line}\r\n"
      "exit /b %ERRORLEVEL%\r\n")
    execute_process(
      COMMAND cmd /S /C "${_securekit_script}"
      RESULT_VARIABLE _securekit_result)
  else()
    execute_process(
      COMMAND ${ARGN}
      RESULT_VARIABLE _securekit_result)
  endif()
  set(${result_variable} "${_securekit_result}" PARENT_SCOPE)
endfunction()

function(_securekit_append_host_configure_args output_variable)
  set(_securekit_args ${${output_variable}})
  if(DEFINED SECUREKIT_CMAKE_GENERATOR AND NOT SECUREKIT_CMAKE_GENERATOR STREQUAL "")
    list(APPEND _securekit_args -G "${SECUREKIT_CMAKE_GENERATOR}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_GENERATOR_PLATFORM AND NOT SECUREKIT_CMAKE_GENERATOR_PLATFORM STREQUAL "")
    list(APPEND _securekit_args -A "${SECUREKIT_CMAKE_GENERATOR_PLATFORM}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_GENERATOR_TOOLSET AND NOT SECUREKIT_CMAKE_GENERATOR_TOOLSET STREQUAL "")
    list(APPEND _securekit_args -T "${SECUREKIT_CMAKE_GENERATOR_TOOLSET}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_MAKE_PROGRAM AND NOT SECUREKIT_CMAKE_MAKE_PROGRAM STREQUAL "")
    list(APPEND _securekit_args "-DCMAKE_MAKE_PROGRAM=${SECUREKIT_CMAKE_MAKE_PROGRAM}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_CXX_COMPILER AND NOT SECUREKIT_CMAKE_CXX_COMPILER STREQUAL "")
    list(APPEND _securekit_args "-DCMAKE_CXX_COMPILER=${SECUREKIT_CMAKE_CXX_COMPILER}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_AR AND NOT SECUREKIT_CMAKE_AR STREQUAL "")
    list(APPEND _securekit_args "-DCMAKE_AR=${SECUREKIT_CMAKE_AR}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_LINKER AND NOT SECUREKIT_CMAKE_LINKER STREQUAL "")
    list(APPEND _securekit_args "-DCMAKE_LINKER=${SECUREKIT_CMAKE_LINKER}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_MT AND NOT SECUREKIT_CMAKE_MT STREQUAL "")
    list(APPEND _securekit_args "-DCMAKE_MT=${SECUREKIT_CMAKE_MT}")
  endif()
  if(DEFINED SECUREKIT_CMAKE_RC_COMPILER AND NOT SECUREKIT_CMAKE_RC_COMPILER STREQUAL "")
    list(APPEND _securekit_args "-DCMAKE_RC_COMPILER=${SECUREKIT_CMAKE_RC_COMPILER}")
  endif()
  if(DEFINED CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE STREQUAL "")
    list(APPEND _securekit_args "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
  endif()
  if(DEFINED VCPKG_TARGET_TRIPLET AND NOT VCPKG_TARGET_TRIPLET STREQUAL "")
    list(APPEND _securekit_args "-DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}")
  endif()
  if(DEFINED OPENSSL_ROOT_DIR AND NOT OPENSSL_ROOT_DIR STREQUAL "")
    list(APPEND _securekit_args "-DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}")
  endif()
  set(${output_variable} ${_securekit_args} PARENT_SCOPE)
endfunction()

set(_securekit_use_msvc_env FALSE)
if(WIN32
    AND DEFINED SECUREKIT_CMAKE_GENERATOR
    AND SECUREKIT_CMAKE_GENERATOR MATCHES "Ninja"
    AND DEFINED SECUREKIT_CMAKE_CXX_COMPILER
    AND SECUREKIT_CMAKE_CXX_COMPILER MATCHES "[/\\\\]cl\\.exe$"
    AND "$ENV{LIB}" STREQUAL "")
  string(REGEX REPLACE "/VC/Tools/MSVC/[^/]+/bin/[^/]+/[^/]+/cl\\.exe$" ""
    _securekit_vs_root "${SECUREKIT_CMAKE_CXX_COMPILER}")
  set(_securekit_vsdevcmd "${_securekit_vs_root}/Common7/Tools/VsDevCmd.bat")
  if(EXISTS "${_securekit_vsdevcmd}")
    message(STATUS "Using VsDevCmd for MSVC Ninja package checks")
    set(_securekit_use_msvc_env TRUE)
  else()
    message(WARNING "VsDevCmd.bat was not found for MSVC Ninja package checks: ${_securekit_vsdevcmd}")
  endif()
endif()

list(GET _securekit_source_artifacts 0 _securekit_source_artifact)
file(MAKE_DIRECTORY "${_securekit_source_extract_dir}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar xf "${_securekit_source_artifact}"
  WORKING_DIRECTORY "${_securekit_source_extract_dir}"
  RESULT_VARIABLE _securekit_source_extract_result
  ERROR_VARIABLE _securekit_source_extract_error)
if(NOT _securekit_source_extract_result EQUAL 0)
  message(FATAL_ERROR "Source package extraction failed: ${_securekit_source_extract_error}")
endif()

file(GLOB _securekit_source_cmakelists LIST_DIRECTORIES FALSE
  "${_securekit_source_extract_dir}/CMakeLists.txt"
  "${_securekit_source_extract_dir}/*/CMakeLists.txt")
list(LENGTH _securekit_source_cmakelists _securekit_source_cmakelists_count)
if(NOT _securekit_source_cmakelists_count EQUAL 1)
  message(FATAL_ERROR "Expected one extracted source tree, found ${_securekit_source_cmakelists_count}")
endif()
list(GET _securekit_source_cmakelists 0 _securekit_source_cmakelist)
get_filename_component(_securekit_extracted_source_dir "${_securekit_source_cmakelist}" DIRECTORY)
list(APPEND _securekit_source_configure_args -S "${_securekit_extracted_source_dir}")
_securekit_append_host_configure_args(_securekit_source_configure_args)

_securekit_run_consumer_command(
  _securekit_source_configure_result
  source-configure
  "${CMAKE_COMMAND}"
  ${_securekit_source_configure_args})
if(NOT _securekit_source_configure_result EQUAL 0)
  message(FATAL_ERROR "Source package configure failed")
endif()

_securekit_run_consumer_command(
  _securekit_source_build_result
  source-build
  "${CMAKE_COMMAND}"
  --build "${_securekit_source_build_dir}"
  --config "${SECUREKIT_BUILD_CONFIG}"
  --target securekit_cli)
if(NOT _securekit_source_build_result EQUAL 0)
  message(FATAL_ERROR "Source package build failed")
endif()

_securekit_run_consumer_command(
  _securekit_source_install_result
  source-install
  "${CMAKE_COMMAND}"
  --install "${_securekit_source_build_dir}"
  --config "${SECUREKIT_BUILD_CONFIG}"
  --prefix "${_securekit_source_install_prefix}")
if(NOT _securekit_source_install_result EQUAL 0)
  message(FATAL_ERROR "Source package install failed")
endif()

set(_securekit_source_cli "${_securekit_source_install_prefix}/bin/securekit${_securekit_exe_suffix}")
if(NOT EXISTS "${_securekit_source_cli}")
  message(FATAL_ERROR "Source package install is missing CLI: ${_securekit_source_cli}")
endif()
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_securekit_path}" "${_securekit_source_cli}" --version
  RESULT_VARIABLE _securekit_source_cli_result
  OUTPUT_VARIABLE _securekit_source_cli_output
  ERROR_VARIABLE _securekit_source_cli_error)
if(NOT _securekit_source_cli_result EQUAL 0)
  message(FATAL_ERROR "Source package CLI smoke failed: ${_securekit_source_cli_error}")
endif()
string(REPLACE "\r\n" "\n" _securekit_source_cli_output "${_securekit_source_cli_output}")
if(NOT _securekit_source_cli_output STREQUAL "securekit ${SECUREKIT_PROJECT_VERSION}\n")
  message(FATAL_ERROR "Source package CLI version mismatch: ${_securekit_source_cli_output}")
endif()

set(_securekit_library_only_configure_args
  -S "${_securekit_extracted_source_dir}"
  -B "${_securekit_library_only_build_dir}"
  -DBUILD_TESTING=OFF
  -DSECUREKIT_BUILD_TESTS=OFF
  -DSECUREKIT_BUILD_CLI=OFF
  -DSECUREKIT_INSTALL_CLI=OFF
  "-DSECUREKIT_WARNINGS_AS_ERRORS=${SECUREKIT_WARNINGS_AS_ERRORS}"
  "-DCMAKE_INSTALL_PREFIX=${_securekit_library_only_install_prefix}"
  "-DCMAKE_BUILD_TYPE=${SECUREKIT_BUILD_CONFIG}")
_securekit_append_host_configure_args(_securekit_library_only_configure_args)

_securekit_run_consumer_command(
  _securekit_library_only_configure_result
  library-only-configure
  "${CMAKE_COMMAND}"
  ${_securekit_library_only_configure_args})
if(NOT _securekit_library_only_configure_result EQUAL 0)
  message(FATAL_ERROR "Library-only package configure failed")
endif()

_securekit_run_consumer_command(
  _securekit_library_only_build_result
  library-only-build
  "${CMAKE_COMMAND}"
  --build "${_securekit_library_only_build_dir}"
  --config "${SECUREKIT_BUILD_CONFIG}"
  --target securekit)
if(NOT _securekit_library_only_build_result EQUAL 0)
  message(FATAL_ERROR "Library-only package build failed")
endif()

_securekit_run_consumer_command(
  _securekit_library_only_install_result
  library-only-install
  "${CMAKE_COMMAND}"
  --install "${_securekit_library_only_build_dir}"
  --config "${SECUREKIT_BUILD_CONFIG}"
  --prefix "${_securekit_library_only_install_prefix}")
if(NOT _securekit_library_only_install_result EQUAL 0)
  message(FATAL_ERROR "Library-only package install failed")
endif()

set(_securekit_library_only_cli "${_securekit_library_only_install_prefix}/bin/securekit${_securekit_exe_suffix}")
if(EXISTS "${_securekit_library_only_cli}")
  message(FATAL_ERROR "Library-only package unexpectedly installed CLI: ${_securekit_library_only_cli}")
endif()

set(_securekit_library_only_consumer_configure_args
  -S "${SECUREKIT_SOURCE_DIR}/tests/consumer"
  -B "${_securekit_library_only_consumer_build_dir}"
  "-DCMAKE_PREFIX_PATH=${_securekit_library_only_install_prefix}"
  "-DSECUREKIT_EXPECTED_VERSION=${SECUREKIT_PROJECT_VERSION}"
  "-DSECUREKIT_EXPECTED_VERSION_MAJOR=${SECUREKIT_PROJECT_VERSION_MAJOR}"
  "-DSECUREKIT_EXPECTED_VERSION_MINOR=${SECUREKIT_PROJECT_VERSION_MINOR}"
  "-DSECUREKIT_EXPECTED_VERSION_PATCH=${SECUREKIT_PROJECT_VERSION_PATCH}"
  "-DCMAKE_BUILD_TYPE=${SECUREKIT_BUILD_CONFIG}")
_securekit_append_host_configure_args(_securekit_library_only_consumer_configure_args)

_securekit_run_consumer_command(
  _securekit_library_only_consumer_configure_result
  library-only-consumer-configure
  "${CMAKE_COMMAND}"
  ${_securekit_library_only_consumer_configure_args})
if(NOT _securekit_library_only_consumer_configure_result EQUAL 0)
  message(FATAL_ERROR "Library-only consumer configure failed")
endif()

_securekit_run_consumer_command(
  _securekit_library_only_consumer_build_result
  library-only-consumer-build
  "${CMAKE_COMMAND}"
  --build "${_securekit_library_only_consumer_build_dir}"
  --config "${SECUREKIT_BUILD_CONFIG}")
if(NOT _securekit_library_only_consumer_build_result EQUAL 0)
  message(FATAL_ERROR "Library-only consumer build failed")
endif()

set(_securekit_library_only_consumer_candidates
  "${_securekit_library_only_consumer_build_dir}/${SECUREKIT_BUILD_CONFIG}/securekit_consumer${_securekit_exe_suffix}"
  "${_securekit_library_only_consumer_build_dir}/securekit_consumer${_securekit_exe_suffix}")

set(_securekit_library_only_consumer_exe "")
foreach(_securekit_library_only_consumer_candidate IN LISTS _securekit_library_only_consumer_candidates)
  if(EXISTS "${_securekit_library_only_consumer_candidate}")
    set(_securekit_library_only_consumer_exe "${_securekit_library_only_consumer_candidate}")
    break()
  endif()
endforeach()
if(_securekit_library_only_consumer_exe STREQUAL "")
  message(FATAL_ERROR "Library-only consumer executable not found under ${_securekit_library_only_consumer_build_dir}")
endif()

set(_securekit_library_only_path "${_securekit_library_only_install_prefix}/bin${_securekit_path_separator}${_securekit_path}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_securekit_library_only_path}" "${_securekit_library_only_consumer_exe}"
  RESULT_VARIABLE _securekit_library_only_consumer_run_result)
if(NOT _securekit_library_only_consumer_run_result EQUAL 0)
  message(FATAL_ERROR "Library-only consumer run failed")
endif()

_securekit_append_host_configure_args(_securekit_consumer_configure_args)

_securekit_run_consumer_command(
  _securekit_consumer_configure_result
  consumer-configure
  "${CMAKE_COMMAND}"
  ${_securekit_consumer_configure_args})
if(NOT _securekit_consumer_configure_result EQUAL 0)
  message(FATAL_ERROR "Consumer configure failed")
endif()

_securekit_run_consumer_command(
  _securekit_consumer_build_result
  consumer-build
  "${CMAKE_COMMAND}"
  --build "${_securekit_consumer_build_dir}"
  --config "${SECUREKIT_BUILD_CONFIG}")
if(NOT _securekit_consumer_build_result EQUAL 0)
  message(FATAL_ERROR "Consumer build failed")
endif()

set(_securekit_consumer_candidates
  "${_securekit_consumer_build_dir}/${SECUREKIT_BUILD_CONFIG}/securekit_consumer${_securekit_exe_suffix}"
  "${_securekit_consumer_build_dir}/securekit_consumer${_securekit_exe_suffix}")

set(_securekit_consumer_exe "")
foreach(_securekit_consumer_candidate IN LISTS _securekit_consumer_candidates)
  if(EXISTS "${_securekit_consumer_candidate}")
    set(_securekit_consumer_exe "${_securekit_consumer_candidate}")
    break()
  endif()
endforeach()
if(_securekit_consumer_exe STREQUAL "")
  message(FATAL_ERROR "Consumer executable not found under ${_securekit_consumer_build_dir}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_securekit_path}" "${_securekit_consumer_exe}"
  RESULT_VARIABLE _securekit_consumer_run_result)
if(NOT _securekit_consumer_run_result EQUAL 0)
  message(FATAL_ERROR "Consumer run failed")
endif()
