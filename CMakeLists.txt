cmake_minimum_required(VERSION 3.20)

project(
  amv
  VERSION 0.1.0
  DESCRIPTION "Awesome Modules Volume 1 crypto utilities"
  LANGUAGES CXX)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)

add_library(amv
  src/aead.cpp
  src/base64.cpp
  src/error.cpp
  src/hash.cpp
  src/hex.cpp
  src/random.cpp)

add_library(amv::amv ALIAS amv)

target_compile_features(amv PUBLIC cxx_std_20)

target_include_directories(amv
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(amv PUBLIC OpenSSL::Crypto)

target_compile_definitions(amv PRIVATE AMV_BUILDING_LIBRARY)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(amv PUBLIC AMV_SHARED)
endif()

if(MSVC)
  target_compile_options(amv PRIVATE /W4 /permissive-)
else()
  target_compile_options(amv PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(AMV_BUILD_TESTS "Build AMV tests" ${BUILD_TESTING})

if(AMV_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(amv_tests
    tests/aead_test.cpp
    tests/base64_test.cpp
    tests/hash_test.cpp
    tests/hex_test.cpp
    tests/public_headers_test.cpp
    tests/random_test.cpp)

  target_link_libraries(amv_tests PRIVATE amv::amv GTest::gtest_main)
  target_compile_features(amv_tests PRIVATE cxx_std_20)

  set(_amv_test_properties)
  if(WIN32)
    set(AMV_OPENSSL_RUNTIME_DIR "" CACHE PATH
      "Directory containing OpenSSL runtime DLLs for AMV tests")
    if(NOT AMV_OPENSSL_RUNTIME_DIR AND DEFINED OPENSSL_ROOT_DIR)
      set(AMV_OPENSSL_RUNTIME_DIR "${OPENSSL_ROOT_DIR}/bin")
    endif()
    if(AMV_OPENSSL_RUNTIME_DIR)
      set(_amv_test_path "${AMV_OPENSSL_RUNTIME_DIR};$ENV{PATH}")
      string(REPLACE ";" "\\;" _amv_test_path "${_amv_test_path}")
      list(APPEND _amv_test_properties ENVIRONMENT "PATH=${_amv_test_path}")
      add_custom_command(TARGET amv_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${AMV_OPENSSL_RUNTIME_DIR}"
          "$<TARGET_FILE_DIR:amv_tests>")
    endif()
  endif()

  include(GoogleTest)
  gtest_discover_tests(amv_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES ${_amv_test_properties})
endif()

install(
  TARGETS amv
  EXPORT amvTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  DIRECTORY include/amv
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT amvTargets
  NAMESPACE amv::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/amv)

configure_package_config_file(
  cmake/amvConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/amvConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/amv)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/amvConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/amvConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/amvConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/amv)
