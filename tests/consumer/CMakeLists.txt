cmake_minimum_required(VERSION 3.20)

project(securekit_consumer LANGUAGES CXX)

find_package(securekit CONFIG REQUIRED)

add_executable(securekit_consumer main.cpp)
target_compile_features(securekit_consumer PRIVATE cxx_std_20)
target_compile_definitions(securekit_consumer PRIVATE
  SECUREKIT_EXPECTED_VERSION="${SECUREKIT_EXPECTED_VERSION}"
  SECUREKIT_EXPECTED_VERSION_MAJOR=${SECUREKIT_EXPECTED_VERSION_MAJOR}
  SECUREKIT_EXPECTED_VERSION_MINOR=${SECUREKIT_EXPECTED_VERSION_MINOR}
  SECUREKIT_EXPECTED_VERSION_PATCH=${SECUREKIT_EXPECTED_VERSION_PATCH})
target_link_libraries(securekit_consumer PRIVATE securekit::securekit)
