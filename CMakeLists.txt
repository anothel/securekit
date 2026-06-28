cmake_minimum_required(VERSION 3.20)

project(
  securekit
  VERSION 0.2.0
  DESCRIPTION "SecureKit security utilities"
  LANGUAGES CXX)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)

option(SECUREKIT_WARNINGS_AS_ERRORS "Treat SecureKit build warnings as errors" OFF)
option(SECUREKIT_BUILD_CLI "Build SecureKit CLI" ON)
option(SECUREKIT_INSTALL_CLI "Install SecureKit CLI" ON)
option(SECUREKIT_BUILD_FUZZ "Build SecureKit libFuzzer targets" OFF)
option(SECUREKIT_ENABLE_COVERAGE "Build with GCC/Clang coverage instrumentation" OFF)

if(SECUREKIT_INSTALL_CLI AND NOT SECUREKIT_BUILD_CLI)
  message(FATAL_ERROR "SECUREKIT_INSTALL_CLI=ON requires SECUREKIT_BUILD_CLI=ON")
endif()

set(_securekit_compile_options)
set(_securekit_coverage_options)
if(MSVC)
  list(APPEND _securekit_compile_options /W4 /permissive-)
  if(SECUREKIT_WARNINGS_AS_ERRORS)
    list(APPEND _securekit_compile_options /WX)
  endif()
  if(SECUREKIT_ENABLE_COVERAGE)
    message(FATAL_ERROR "SECUREKIT_ENABLE_COVERAGE=ON requires GCC or Clang")
  endif()
else()
  list(APPEND _securekit_compile_options -Wall -Wextra -Wpedantic)
  if(SECUREKIT_WARNINGS_AS_ERRORS)
    list(APPEND _securekit_compile_options -Werror)
  endif()
  if(SECUREKIT_ENABLE_COVERAGE)
    list(APPEND _securekit_coverage_options --coverage -O0 -g)
  endif()
endif()

find_program(CLANG_FORMAT_EXE
  NAMES clang-format
  HINTS
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/bin")
if(CLANG_FORMAT_EXE)
  file(GLOB_RECURSE _securekit_format_sources CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/securekit/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp)
  add_custom_target(format-check
    COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${_securekit_format_sources}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL)
else()
  add_custom_target(format-check
    COMMAND ${CMAKE_COMMAND} -E echo "format-check requires clang-format on PATH"
    COMMAND ${CMAKE_COMMAND} -E false
    USES_TERMINAL)
endif()

add_library(securekit
  src/aead.cpp
  src/base64.cpp
  src/compare.cpp
  src/error.cpp
  src/file_crypto.cpp
  src/file.cpp
  src/file_io.cpp
  src/hash.cpp
  src/hex.cpp
  src/key_wrap.cpp
  src/packet_stream.cpp
  src/random.cpp
  src/version.cpp)

add_library(securekit::securekit ALIAS securekit)

target_compile_features(securekit PUBLIC cxx_std_20)

target_include_directories(securekit
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(securekit PUBLIC OpenSSL::Crypto)

target_compile_definitions(securekit PRIVATE
  SECUREKIT_BUILDING_LIBRARY
  SECUREKIT_VERSION="${PROJECT_VERSION}"
  SECUREKIT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  SECUREKIT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  SECUREKIT_VERSION_PATCH=${PROJECT_VERSION_PATCH})

if(BUILD_SHARED_LIBS)
  target_compile_definitions(securekit PUBLIC SECUREKIT_SHARED)
endif()

target_compile_options(securekit PRIVATE ${_securekit_compile_options} ${_securekit_coverage_options})
target_link_options(securekit PUBLIC ${_securekit_coverage_options})

if(SECUREKIT_BUILD_CLI)
  add_executable(securekit_cli
    src/cli/commands.cpp
    src/cli/main.cpp)

  set_target_properties(securekit_cli PROPERTIES OUTPUT_NAME securekit)

  target_link_libraries(securekit_cli PRIVATE securekit::securekit)
  target_compile_features(securekit_cli PRIVATE cxx_std_20)

  target_compile_options(securekit_cli PRIVATE ${_securekit_compile_options} ${_securekit_coverage_options})
endif()

add_executable(securekit_example_basic EXCLUDE_FROM_ALL
  examples/basic/main.cpp)
target_link_libraries(securekit_example_basic PRIVATE securekit::securekit)
target_compile_features(securekit_example_basic PRIVATE cxx_std_20)
target_compile_options(securekit_example_basic PRIVATE ${_securekit_compile_options} ${_securekit_coverage_options})

add_custom_target(examples-check
  COMMAND $<TARGET_FILE:securekit_example_basic>
  DEPENDS securekit_example_basic
  USES_TERMINAL)

add_executable(securekit_benchmarks EXCLUDE_FROM_ALL
  benchmarks/crypto_file_bench.cpp)
target_link_libraries(securekit_benchmarks PRIVATE securekit::securekit)
target_compile_features(securekit_benchmarks PRIVATE cxx_std_20)
target_compile_options(securekit_benchmarks PRIVATE ${_securekit_compile_options} ${_securekit_coverage_options})

add_custom_target(benchmarks-check
  COMMAND $<TARGET_FILE:securekit_benchmarks>
  DEPENDS securekit_benchmarks
  USES_TERMINAL)

if(SECUREKIT_BUILD_FUZZ)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "SECUREKIT_BUILD_FUZZ=ON requires Clang with libFuzzer support")
  endif()

  function(securekit_add_fuzz target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE securekit::securekit)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_compile_options(${target} PRIVATE -fsanitize=fuzzer,address ${_securekit_compile_options})
    target_link_options(${target} PRIVATE -fsanitize=fuzzer,address)
  endfunction()

  securekit_add_fuzz(securekit_fuzz_hex tests/fuzz/hex_fuzz.cpp)
  securekit_add_fuzz(securekit_fuzz_base64 tests/fuzz/base64_fuzz.cpp)
  securekit_add_fuzz(securekit_fuzz_base64url tests/fuzz/base64url_fuzz.cpp)
  securekit_add_fuzz(securekit_fuzz_skt1 tests/fuzz/skt1_fuzz.cpp)
  securekit_add_fuzz(securekit_fuzz_file tests/fuzz/file_fuzz.cpp)

  set(_securekit_fuzz_corpus
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/fuzz/corpus)

  add_custom_target(fuzz-smoke
    COMMAND $<TARGET_FILE:securekit_fuzz_hex> -runs=1 ${_securekit_fuzz_corpus}
    COMMAND $<TARGET_FILE:securekit_fuzz_base64> -runs=1 ${_securekit_fuzz_corpus}
    COMMAND $<TARGET_FILE:securekit_fuzz_base64url> -runs=1 ${_securekit_fuzz_corpus}
    COMMAND $<TARGET_FILE:securekit_fuzz_skt1> -runs=1 ${_securekit_fuzz_corpus}
    COMMAND $<TARGET_FILE:securekit_fuzz_file> -runs=1 ${_securekit_fuzz_corpus}
    DEPENDS
      securekit_fuzz_hex
      securekit_fuzz_base64
      securekit_fuzz_base64url
      securekit_fuzz_skt1
      securekit_fuzz_file
    USES_TERMINAL)
endif()

if(SECUREKIT_BUILD_CLI AND SECUREKIT_INSTALL_CLI)
  add_custom_target(package-check
    COMMAND ${CMAKE_COMMAND}
      -DSECUREKIT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
      -DSECUREKIT_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -DSECUREKIT_PACKAGE_CHECK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/package-check
      -DSECUREKIT_PROJECT_NAME=${PROJECT_NAME}
      -DSECUREKIT_PROJECT_VERSION=${PROJECT_VERSION}
      -DSECUREKIT_PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
      -DSECUREKIT_PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
      -DSECUREKIT_PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
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
      -DSECUREKIT_WARNINGS_AS_ERRORS=${SECUREKIT_WARNINGS_AS_ERRORS}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
      -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
      -DSECUREKIT_OPENSSL_RUNTIME_DIR=${SECUREKIT_OPENSSL_RUNTIME_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/package/check_package.cmake
    DEPENDS securekit securekit_cli
    USES_TERMINAL)
else()
  add_custom_target(package-check
    COMMAND ${CMAKE_COMMAND} -E echo "package-check requires SECUREKIT_BUILD_CLI=ON and SECUREKIT_INSTALL_CLI=ON because release validation requires installed CLI"
    COMMAND ${CMAKE_COMMAND} -E false
    USES_TERMINAL)
endif()

if(SECUREKIT_BUILD_CLI AND SECUREKIT_INSTALL_CLI)
  add_custom_target(dogfood-check
    COMMAND ${CMAKE_COMMAND}
      -DSECUREKIT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
      -DSECUREKIT_PACKAGE_CHECK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/package-check
      -DSECUREKIT_PROJECT_NAME=${PROJECT_NAME}
      -DSECUREKIT_PROJECT_VERSION=${PROJECT_VERSION}
      -DSECUREKIT_PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
      -DSECUREKIT_PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
      -DSECUREKIT_PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
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
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
      -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
      -DSECUREKIT_OPENSSL_RUNTIME_DIR=${SECUREKIT_OPENSSL_RUNTIME_DIR}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/package/check_dogfood.cmake
    DEPENDS package-check
    USES_TERMINAL)
else()
  add_custom_target(dogfood-check
    COMMAND ${CMAKE_COMMAND} -E echo "dogfood-check requires SECUREKIT_BUILD_CLI=ON and SECUREKIT_INSTALL_CLI=ON"
    COMMAND ${CMAKE_COMMAND} -E false
    USES_TERMINAL)
endif()

add_custom_target(release-workflow-check
  COMMAND ${CMAKE_COMMAND}
    -DSECUREKIT_WORKFLOW_FILE=${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/securekit-ci.yml
    -DSECUREKIT_CODEQL_WORKFLOW_FILE=${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/codeql.yml
    -DSECUREKIT_PROJECT_VERSION:STRING=${PROJECT_VERSION}
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
    tests/internal_wipe_test.cpp
    tests/key_wrap_test.cpp
    tests/packet_stream_test.cpp
    tests/public_headers_test.cpp
    tests/random_test.cpp)

  target_link_libraries(securekit_tests PRIVATE securekit::securekit GTest::gtest_main)
  target_compile_features(securekit_tests PRIVATE cxx_std_20)
  target_compile_options(securekit_tests PRIVATE ${_securekit_compile_options} ${_securekit_coverage_options})
  target_compile_definitions(securekit_tests PRIVATE
    SECUREKIT_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
    SECUREKIT_FORMAT_DOC="${CMAKE_CURRENT_SOURCE_DIR}/docs/FORMAT.md"
    SECUREKIT_EXPECTED_VERSION="${PROJECT_VERSION}"
    SECUREKIT_EXPECTED_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    SECUREKIT_EXPECTED_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    SECUREKIT_EXPECTED_VERSION_PATCH=${PROJECT_VERSION_PATCH})

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

  set(_securekit_check_dependencies securekit_tests)
  if(TARGET securekit_cli)
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
    list(APPEND _securekit_check_dependencies securekit_cli)
  endif()

  add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
      --test-dir ${CMAKE_CURRENT_BINARY_DIR}
      --build-config $<CONFIG>
      --output-on-failure
    DEPENDS ${_securekit_check_dependencies}
    USES_TERMINAL)

  find_program(GCOVR_EXE NAMES gcovr)
  if(SECUREKIT_ENABLE_COVERAGE AND GCOVR_EXE)
    add_custom_target(coverage-report
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/coverage
      COMMAND ${GCOVR_EXE}
        --root ${CMAKE_CURRENT_SOURCE_DIR}
        --object-directory ${CMAKE_CURRENT_BINARY_DIR}
        --filter ${CMAKE_CURRENT_SOURCE_DIR}/include
        --filter ${CMAKE_CURRENT_SOURCE_DIR}/src
        --exclude ${CMAKE_CURRENT_SOURCE_DIR}/tests
        --html-details ${CMAKE_CURRENT_BINARY_DIR}/coverage/securekit.html
        --xml ${CMAKE_CURRENT_BINARY_DIR}/coverage/coverage.xml
        --print-summary
      DEPENDS check
      USES_TERMINAL)
  else()
    add_custom_target(coverage-report
      COMMAND ${CMAKE_COMMAND} -E echo "coverage-report requires -DSECUREKIT_ENABLE_COVERAGE=ON, GCC or Clang, and gcovr"
      COMMAND ${CMAKE_COMMAND} -E false
      USES_TERMINAL)
  endif()
endif()

if(TARGET check AND SECUREKIT_BUILD_CLI AND SECUREKIT_INSTALL_CLI)
  add_custom_target(release-preflight
    COMMAND ${CMAKE_COMMAND}
      -DSECUREKIT_PACKAGE_CHECK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/package-check
      -DSECUREKIT_PROJECT_NAME=${PROJECT_NAME}
      -DSECUREKIT_PROJECT_VERSION:STRING=${PROJECT_VERSION}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/package/check_release_assets.cmake
    COMMAND ${CMAKE_COMMAND}
      -DSECUREKIT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
      -DSECUREKIT_PACKAGE_CHECK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/package-check
      -DSECUREKIT_PROJECT_NAME=${PROJECT_NAME}
      -DSECUREKIT_PROJECT_VERSION:STRING=${PROJECT_VERSION}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/package/check_release_preflight.cmake
    DEPENDS
      check
      benchmarks-check
      examples-check
      dogfood-check
      release-workflow-check
    USES_TERMINAL)
elseif(TARGET check)
  add_custom_target(release-preflight
    COMMAND ${CMAKE_COMMAND} -E echo "release-preflight requires SECUREKIT_BUILD_CLI=ON and SECUREKIT_INSTALL_CLI=ON because release validation requires installed CLI"
    COMMAND ${CMAKE_COMMAND} -E false
    USES_TERMINAL)
else()
  add_custom_target(release-preflight
    COMMAND ${CMAKE_COMMAND} -E echo "release-preflight requires SECUREKIT_BUILD_TESTS=ON"
    COMMAND ${CMAKE_COMMAND} -E false
    USES_TERMINAL)
endif()

install(
  TARGETS securekit
  EXPORT securekitTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(SECUREKIT_INSTALL_CLI)
  install(
    TARGETS securekit_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

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
  "/\\.agents/"
  "/\\.codex/"
  "/\\.git/"
  "/\\.github/"
  "/\\.vscode/"
  "/build[^/]*/"
  "/docs/superpowers/"
  "/securekit-cli-[^/]*$"
  "/package-check/"
  "/_CPack_Packages/"
  "CMakeCache\\.txt$"
  "CMakeFiles/"
  "\\.user$")
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)
