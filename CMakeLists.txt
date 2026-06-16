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

if(MSVC)
  target_compile_options(securekit_cli PRIVATE /W4 /permissive-)
else()
  target_compile_options(securekit_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

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
    tests/hash_test.cpp
    tests/hex_test.cpp
    tests/key_wrap_test.cpp
    tests/packet_stream_test.cpp
    tests/public_headers_test.cpp
    tests/random_test.cpp)

  target_link_libraries(securekit_tests PRIVATE securekit::securekit GTest::gtest_main)
  target_compile_features(securekit_tests PRIVATE cxx_std_20)

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
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli/check_cli.cmake)
  if(_securekit_test_properties)
    set_tests_properties(Cli.UtilityCommands PROPERTIES ${_securekit_test_properties})
  endif()
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
