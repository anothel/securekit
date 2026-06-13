cmake_minimum_required(VERSION 3.20)

project(securekit_consumer LANGUAGES CXX)

find_package(securekit CONFIG REQUIRED)

add_executable(securekit_consumer main.cpp)
target_compile_features(securekit_consumer PRIVATE cxx_std_20)
target_link_libraries(securekit_consumer PRIVATE securekit::securekit)
