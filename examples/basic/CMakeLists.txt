cmake_minimum_required(VERSION 3.20)

project(securekit_basic_example LANGUAGES CXX)

find_package(securekit CONFIG REQUIRED)

add_executable(securekit_basic_example main.cpp)
target_compile_features(securekit_basic_example PRIVATE cxx_std_20)
target_link_libraries(securekit_basic_example PRIVATE securekit::securekit)
