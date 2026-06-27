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

if(WIN32)
  set(_securekit_exe_suffix ".exe")
  set(_securekit_path_separator ";")
else()
  set(_securekit_exe_suffix "")
  set(_securekit_path_separator ":")
endif()

set(_securekit_artifact_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/artifacts")
set(_securekit_dogfood_root "${SECUREKIT_PACKAGE_CHECK_ROOT}/dogfood")
set(_securekit_extract_dir "${_securekit_dogfood_root}/extract")
set(_securekit_work_dir "${_securekit_dogfood_root}/work")
set(_securekit_consumer_src_dir "${_securekit_dogfood_root}/consumer-src")
set(_securekit_consumer_build_dir "${_securekit_dogfood_root}/consumer-build")

file(GLOB _securekit_binary_archives
  "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-*.zip"
  "${_securekit_artifact_dir}/${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-*.tar.gz")
list(FILTER _securekit_binary_archives EXCLUDE REGEX "-source\\.")
list(SORT _securekit_binary_archives)

list(LENGTH _securekit_binary_archives _securekit_binary_archive_count)
if(_securekit_binary_archive_count EQUAL 0)
  message(FATAL_ERROR "No binary archive found under ${_securekit_artifact_dir}; run package-check first")
endif()
list(GET _securekit_binary_archives 0 _securekit_binary_archive)

file(REMOVE_RECURSE "${_securekit_dogfood_root}")
file(MAKE_DIRECTORY "${_securekit_extract_dir}")
file(MAKE_DIRECTORY "${_securekit_work_dir}")
file(MAKE_DIRECTORY "${_securekit_consumer_src_dir}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar xf "${_securekit_binary_archive}"
  WORKING_DIRECTORY "${_securekit_extract_dir}"
  RESULT_VARIABLE _securekit_extract_result
  ERROR_VARIABLE _securekit_extract_error)
if(NOT _securekit_extract_result EQUAL 0)
  message(FATAL_ERROR "Dogfood archive extraction failed: ${_securekit_extract_error}")
endif()

file(GLOB_RECURSE _securekit_config_files LIST_DIRECTORIES FALSE
  "${_securekit_extract_dir}/*/lib/cmake/securekit/securekitConfig.cmake"
  "${_securekit_extract_dir}/lib/cmake/securekit/securekitConfig.cmake")
list(LENGTH _securekit_config_files _securekit_config_file_count)
if(NOT _securekit_config_file_count EQUAL 1)
  message(FATAL_ERROR "Expected one extracted securekitConfig.cmake, found ${_securekit_config_file_count}")
endif()
list(GET _securekit_config_files 0 _securekit_config_file)
get_filename_component(_securekit_package_dir "${_securekit_config_file}" DIRECTORY)
get_filename_component(_securekit_package_dir "${_securekit_package_dir}" DIRECTORY)
get_filename_component(_securekit_package_dir "${_securekit_package_dir}" DIRECTORY)
get_filename_component(_securekit_package_prefix "${_securekit_package_dir}" DIRECTORY)

set(_securekit_cli "${_securekit_package_prefix}/bin/securekit${_securekit_exe_suffix}")
if(NOT EXISTS "${_securekit_cli}")
  message(FATAL_ERROR "Extracted package is missing CLI: ${_securekit_cli}")
endif()

set(_securekit_path "${_securekit_package_prefix}/bin")
if(WIN32)
  if(DEFINED SECUREKIT_OPENSSL_RUNTIME_DIR AND NOT SECUREKIT_OPENSSL_RUNTIME_DIR STREQUAL "")
    string(APPEND _securekit_path "${_securekit_path_separator}${SECUREKIT_OPENSSL_RUNTIME_DIR}")
  elseif(DEFINED OPENSSL_ROOT_DIR AND NOT OPENSSL_ROOT_DIR STREQUAL "")
    string(APPEND _securekit_path "${_securekit_path_separator}${OPENSSL_ROOT_DIR}/bin")
  endif()
endif()
string(APPEND _securekit_path "${_securekit_path_separator}$ENV{PATH}")

function(_securekit_dogfood_cli description expected_stdout)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_securekit_path}" "${_securekit_cli}" ${ARGN}
    RESULT_VARIABLE _securekit_result
    OUTPUT_VARIABLE _securekit_stdout
    ERROR_VARIABLE _securekit_stderr)
  if(NOT _securekit_result EQUAL 0)
    message(FATAL_ERROR "${description} failed: ${_securekit_stderr}")
  endif()
  string(REPLACE "\r\n" "\n" _securekit_stdout "${_securekit_stdout}")
  if(NOT _securekit_stdout STREQUAL "${expected_stdout}")
    message(FATAL_ERROR "${description} stdout mismatch: [${_securekit_stdout}]")
  endif()
endfunction()

set(_securekit_plain "${_securekit_work_dir}/plain.txt")
set(_securekit_key "${_securekit_work_dir}/key.hex")
set(_securekit_sealed "${_securekit_work_dir}/plain.skf")
set(_securekit_opened "${_securekit_work_dir}/opened.txt")
set(_securekit_password "${_securekit_work_dir}/password.txt")
set(_securekit_password_sealed "${_securekit_work_dir}/plain.skp")
set(_securekit_password_opened "${_securekit_work_dir}/password-opened.txt")

file(WRITE "${_securekit_plain}" "dogfood plaintext\n")
file(WRITE "${_securekit_password}" "dogfood password\n")

_securekit_dogfood_cli("keygen" "" keygen --out "${_securekit_key}")
file(READ "${_securekit_key}" _securekit_key_text)
string(STRIP "${_securekit_key_text}" _securekit_key_text)
string(LENGTH "${_securekit_key_text}" _securekit_key_length)
if(NOT _securekit_key_length EQUAL 64 OR NOT _securekit_key_text MATCHES "^[0-9a-f]+$")
  message(FATAL_ERROR "keygen did not write a 64-hex key: [${_securekit_key_text}]")
endif()

_securekit_dogfood_cli("seal-file" "" seal-file --in "${_securekit_plain}" --out "${_securekit_sealed}" --key-file "${_securekit_key}" --aad-text dogfood:v1)
_securekit_dogfood_cli("verify-file" "" verify-file --in "${_securekit_sealed}" --key-file "${_securekit_key}" --aad-text dogfood:v1)
_securekit_dogfood_cli("open-file" "" open-file --in "${_securekit_sealed}" --out "${_securekit_opened}" --key-file "${_securekit_key}" --aad-text dogfood:v1)
file(READ "${_securekit_opened}" _securekit_opened_text)
if(NOT _securekit_opened_text STREQUAL "dogfood plaintext\n")
  message(FATAL_ERROR "open-file did not recover plaintext: [${_securekit_opened_text}]")
endif()

_securekit_dogfood_cli("seal-file-password" "" seal-file-password --in "${_securekit_plain}" --out "${_securekit_password_sealed}" --password-file "${_securekit_password}" --aad-text dogfood:v1)
_securekit_dogfood_cli("verify-file-password" "" verify-file-password --in "${_securekit_password_sealed}" --password-file "${_securekit_password}" --aad-text dogfood:v1)
_securekit_dogfood_cli("open-file-password" "" open-file-password --in "${_securekit_password_sealed}" --out "${_securekit_password_opened}" --password-file "${_securekit_password}" --aad-text dogfood:v1)
file(READ "${_securekit_password_opened}" _securekit_password_opened_text)
if(NOT _securekit_password_opened_text STREQUAL "dogfood plaintext\n")
  message(FATAL_ERROR "open-file-password did not recover plaintext: [${_securekit_password_opened_text}]")
endif()

file(COPY
  "${SECUREKIT_SOURCE_DIR}/tests/consumer/CMakeLists.txt"
  "${SECUREKIT_SOURCE_DIR}/tests/consumer/main.cpp"
  DESTINATION "${_securekit_consumer_src_dir}")

set(_securekit_consumer_configure_args
  -S "${_securekit_consumer_src_dir}"
  -B "${_securekit_consumer_build_dir}"
  "-DCMAKE_PREFIX_PATH=${_securekit_package_prefix}"
  "-DSECUREKIT_EXPECTED_VERSION=${SECUREKIT_PROJECT_VERSION}"
  "-DSECUREKIT_EXPECTED_VERSION_MAJOR=${SECUREKIT_PROJECT_VERSION_MAJOR}"
  "-DSECUREKIT_EXPECTED_VERSION_MINOR=${SECUREKIT_PROJECT_VERSION_MINOR}"
  "-DSECUREKIT_EXPECTED_VERSION_PATCH=${SECUREKIT_PROJECT_VERSION_PATCH}"
  "-DCMAKE_BUILD_TYPE=${SECUREKIT_BUILD_CONFIG}")

if(DEFINED SECUREKIT_CMAKE_GENERATOR AND NOT SECUREKIT_CMAKE_GENERATOR STREQUAL "")
  list(APPEND _securekit_consumer_configure_args -G "${SECUREKIT_CMAKE_GENERATOR}")
endif()
if(DEFINED SECUREKIT_CMAKE_GENERATOR_PLATFORM AND NOT SECUREKIT_CMAKE_GENERATOR_PLATFORM STREQUAL "")
  list(APPEND _securekit_consumer_configure_args -A "${SECUREKIT_CMAKE_GENERATOR_PLATFORM}")
endif()
if(DEFINED SECUREKIT_CMAKE_GENERATOR_TOOLSET AND NOT SECUREKIT_CMAKE_GENERATOR_TOOLSET STREQUAL "")
  list(APPEND _securekit_consumer_configure_args -T "${SECUREKIT_CMAKE_GENERATOR_TOOLSET}")
endif()
foreach(_securekit_host_var IN ITEMS
    CMAKE_MAKE_PROGRAM
    CMAKE_CXX_COMPILER
    CMAKE_AR
    CMAKE_LINKER
    CMAKE_MT
    CMAKE_RC_COMPILER
    CMAKE_TOOLCHAIN_FILE
    VCPKG_TARGET_TRIPLET
    OPENSSL_ROOT_DIR)
  if(DEFINED SECUREKIT_${_securekit_host_var} AND NOT SECUREKIT_${_securekit_host_var} STREQUAL "")
    list(APPEND _securekit_consumer_configure_args "-D${_securekit_host_var}=${SECUREKIT_${_securekit_host_var}}")
  elseif(DEFINED ${_securekit_host_var} AND NOT ${_securekit_host_var} STREQUAL "")
    list(APPEND _securekit_consumer_configure_args "-D${_securekit_host_var}=${${_securekit_host_var}}")
  endif()
endforeach()

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

function(_securekit_run_host_command result_variable script_stem)
  if(_securekit_use_msvc_env)
    _securekit_join_cmdline(_securekit_command_line ${ARGN})
    set(_securekit_script "${_securekit_dogfood_root}/${script_stem}.cmd")
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
    set(_securekit_use_msvc_env TRUE)
  endif()
endif()

_securekit_run_host_command(
  _securekit_consumer_configure_result
  dogfood-consumer-configure
  "${CMAKE_COMMAND}"
  ${_securekit_consumer_configure_args})
if(NOT _securekit_consumer_configure_result EQUAL 0)
  message(FATAL_ERROR "Dogfood consumer configure failed")
endif()

_securekit_run_host_command(
  _securekit_consumer_build_result
  dogfood-consumer-build
  "${CMAKE_COMMAND}"
  --build "${_securekit_consumer_build_dir}"
  --config "${SECUREKIT_BUILD_CONFIG}")
if(NOT _securekit_consumer_build_result EQUAL 0)
  message(FATAL_ERROR "Dogfood consumer build failed")
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
  message(FATAL_ERROR "Dogfood consumer executable not found under ${_securekit_consumer_build_dir}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_securekit_path}" "${_securekit_consumer_exe}"
  RESULT_VARIABLE _securekit_consumer_run_result)
if(NOT _securekit_consumer_run_result EQUAL 0)
  message(FATAL_ERROR "Dogfood consumer run failed")
endif()

file(WRITE "${_securekit_dogfood_root}/DOGFOOD_RESULT.txt"
  "SecureKit ${SECUREKIT_PROJECT_VERSION} dogfood check passed.\n"
  "Archive: ${_securekit_binary_archive}\n"
  "CLI flows: keygen, seal-file/open-file/verify-file, password file flow.\n"
  "C++ API: copied external consumer configured, built, and ran from extracted package.\n")
