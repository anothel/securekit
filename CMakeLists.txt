cmake_minimum_required(VERSION 3.20)

project(
  securekit
  VERSION 0.1.0
  DESCRIPTION "SecureKit security utilities"
  LANGUAGES CXX)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)

add_library(securekit
  src/aead.cpp
  src/base64.cpp
  src/compare.cpp
  src/error.cpp
  src/file.cpp
  src/hash.cpp
  src/hex.cpp
  src/key_wrap.cpp
  src/packet_stream.cpp
  src/random.cpp)

add_library(securekit::securekit ALIAS securekit)

target_compile_features(securekit PUBLIC cxx_std_20)

target_include_directories(securekit
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(securekit PUBLIC OpenSSL::Crypto)

target_compile_definitions(securekit PRIVATE SECUREKIT_BUILDING_LIBRARY)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(securekit PUBLIC SECUREKIT_SHARED)
endif()

if(MSVC)
  target_compile_options(securekit PRIVATE /W4 /permissive-)
else()
  target_compile_options(securekit PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(securekit_cli
  src/cli/main.cpp)

set_target_properties(securekit_cli PROPERTIES OUTPUT_NAME securekit)

target_link_libraries(securekit_cli PRIVATE securekit::securekit)
target_compile_features(securekit_cli PRIVATE cxx_std_20)
target_compile_definitions(securekit_cli PRIVATE SECUREKIT_VERSION="${PROJECT_VERSION}")

if(MSVC)
  target_compile_options(securekit_cli PRIVATE /W4 /permissive-)
else()
  target_compile_options(securekit_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_target(package-check
  COMMAND ${CMAKE_COMMAND}
    -DSECUREKIT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DSECUREKIT_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -DSECUREKIT_PACKAGE_CHECK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/package-check
    -DSECUREKIT_PROJECT_NAME=${PROJECT_NAME}
    -DSECUREKIT_PROJECT_VERSION=${PROJECT_VERSION}
    -DSECUREKIT_BUILD_CONFIG=$<CONFIG>
    -DSECUREKIT_DEFAULT_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DSECUREKIT_CMAKE_GENERATOR=${CMAKE_GENERATOR}
    -DSECUREKIT_CMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
    -DSECUREKIT_CMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
    -DSECUREKIT_CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -DSECUREKIT_CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DSECUREKIT_CMAKE_AR=${CMAKE_AR}
    -DSECUREKIT_CMAKE_LINKER=${CMAKE_LINKER}
    -DSECUREKIT_CMAKE_MT=${CMAKE_MT}
    -DSECUREKIT_CMAKE_RC_COMPILER=${CMAKE_RC_COMPILER}
    -DSECUREKIT_CPACK_COMMAND=${CMAKE_CPACK_COMMAND}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
    -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
    -DSECUREKIT_OPENSSL_RUNTIME_DIR=${SECUREKIT_OPENSSL_RUNTIME_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/package/check_package.cmake
  DEPENDS securekit securekit_cli
  USES_TERMINAL)

add_custom_target(release-workflow-check
  COMMAND ${CMAKE_COMMAND}
    -DSECUREKIT_WORKFLOW_FILE=${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/securekit-ci.yml
    -DSECUREKIT_CODEQL_WORKFLOW_FILE=${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/codeql.yml
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/package/check_release_workflow.cmake
  USES_TERMINAL)

option(SECUREKIT_BUILD_TESTS "Build SecureKit tests" ${BUILD_TESTING})

if(SECUREKIT_BUILD_TESTS)
  find_package(GTest CONFIG QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(_securekit_build_shared_libs "${BUILD_SHARED_LIBS}")
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_MakeAvailable(googletest)
    set(BUILD_SHARED_LIBS "${_securekit_build_shared_libs}")
  endif()

  add_executable(securekit_tests
    tests/aead_test.cpp
    tests/base64_test.cpp
    tests/compare_test.cpp
    tests/file_test.cpp
    tests/fixture_inventory_test.cpp
    tests/hash_test.cpp
    tests/hex_test.cpp
    tests/key_wrap_test.cpp
    tests/packet_stream_test.cpp
    tests/public_headers_test.cpp
    tests/random_test.cpp)

  target_link_libraries(securekit_tests PRIVATE securekit::securekit GTest::gtest_main)
  target_compile_features(securekit_tests PRIVATE cxx_std_20)
  target_compile_definitions(securekit_tests PRIVATE
    SECUREKIT_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")

  set(_securekit_test_properties)
  if(WIN32)
    set(SECUREKIT_OPENSSL_RUNTIME_DIR "" CACHE PATH
      "Directory containing OpenSSL runtime DLLs for SecureKit tests")
    if(NOT SECUREKIT_OPENSSL_RUNTIME_DIR AND DEFINED OPENSSL_ROOT_DIR)
      set(SECUREKIT_OPENSSL_RUNTIME_DIR "${OPENSSL_ROOT_DIR}/bin")
    endif()
    if(SECUREKIT_OPENSSL_RUNTIME_DIR)
      set(_securekit_test_path "${SECUREKIT_OPENSSL_RUNTIME_DIR};$ENV{PATH}")
      string(REPLACE ";" "\\;" _securekit_test_path "${_securekit_test_path}")
      list(APPEND _securekit_test_properties ENVIRONMENT "PATH=${_securekit_test_path}")
      add_custom_command(TARGET securekit_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${SECUREKIT_OPENSSL_RUNTIME_DIR}"
          "$<TARGET_FILE_DIR:securekit_tests>")
    endif()
  endif()

  include(GoogleTest)
  gtest_discover_tests(securekit_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES ${_securekit_test_properties})

  add_test(
    NAME Cli.UtilityCommands
    COMMAND ${CMAKE_COMMAND}
      -DSECUREKIT_CLI=$<TARGET_FILE:securekit_cli>
      -DSECUREKIT_EXPECTED_VERSION=${PROJECT_VERSION}
      -DSECUREKIT_TEST_FIXTURE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli/check_cli.cmake)
  if(_securekit_test_properties)
    set_tests_properties(Cli.UtilityCommands PROPERTIES ${_securekit_test_properties})
  endif()

  add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
      --test-dir ${CMAKE_CURRENT_BINARY_DIR}
      --build-config $<CONFIG>
      --output-on-failure
    DEPENDS securekit_cli securekit_tests
    USES_TERMINAL)
endif()

install(
  TARGETS securekit
  EXPORT securekitTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  TARGETS securekit_cli
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
  DIRECTORY include/securekit
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT securekitTargets
  NAMESPACE securekit::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/securekit)

configure_package_config_file(
  cmake/securekitConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/securekitConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/securekit)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/securekitConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/securekitConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/securekitConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/securekit)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "SecureKit")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "SecureKit maintainers")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME
  "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-source")
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  "/\\.git/"
  "/\\.github/"
  "/\\.vscode/"
  "/build[^/]*/"
  "/package-check/"
  "/_CPack_Packages/"
  "CMakeCache\\.txt$"
  "CMakeFiles/"
  "\\.user$")
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)
