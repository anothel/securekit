if(NOT DEFINED SECUREKIT_SOURCE_DIR)
  message(FATAL_ERROR "SECUREKIT_SOURCE_DIR is required")
endif()

if(NOT DEFINED SECUREKIT_BUILD_DIR)
  message(FATAL_ERROR "SECUREKIT_BUILD_DIR is required")
endif()

if(NOT DEFINED SECUREKIT_PACKAGE_CHECK_ROOT)
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT is required")
endif()

if(NOT DEFINED SECUREKIT_BUILD_CONFIG OR SECUREKIT_BUILD_CONFIG STREQUAL "")
  if(DEFINED SECUREKIT_DEFAULT_BUILD_TYPE AND NOT SECUREKIT_DEFAULT_BUILD_TYPE STREQUAL "")
    set(SECUREKIT_BUILD_CONFIG "${SECUREKIT_DEFAULT_BUILD_TYPE}")
  else()
    set(SECUREKIT_BUILD_CONFIG "Release")
  endif()
endif()

set(_securekit_install_prefix "${SECUREKIT_PACKAGE_CHECK_ROOT}/install")
set(_securekit_consumer_build_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/consumer-build")

file(REMOVE_RECURSE "${_securekit_install_prefix}")
file(REMOVE_RECURSE "${_securekit_consumer_build_dir}")
file(MAKE_DIRECTORY "${SECUREKIT_PACKAGE_CHECK_ROOT}")

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

set(_securekit_consumer_configure_args
  -S "${SECUREKIT_SOURCE_DIR}/tests/consumer"
  -B "${_securekit_consumer_build_dir}"
  "-DCMAKE_PREFIX_PATH=${_securekit_install_prefix}"
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
if(DEFINED CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE STREQUAL "")
  list(APPEND _securekit_consumer_configure_args "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
endif()
if(DEFINED VCPKG_TARGET_TRIPLET AND NOT VCPKG_TARGET_TRIPLET STREQUAL "")
  list(APPEND _securekit_consumer_configure_args "-DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}")
endif()
if(DEFINED OPENSSL_ROOT_DIR AND NOT OPENSSL_ROOT_DIR STREQUAL "")
  list(APPEND _securekit_consumer_configure_args "-DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" ${_securekit_consumer_configure_args}
  RESULT_VARIABLE _securekit_consumer_configure_result)
if(NOT _securekit_consumer_configure_result EQUAL 0)
  message(FATAL_ERROR "Consumer configure failed")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${_securekit_consumer_build_dir}" --config "${SECUREKIT_BUILD_CONFIG}"
  RESULT_VARIABLE _securekit_consumer_build_result)
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
