cmake_minimum_required(VERSION 3.20)

project(amv_consumer LANGUAGES CXX)

find_package(amv CONFIG REQUIRED)

add_executable(amv_consumer main.cpp)
target_compile_features(amv_consumer PRIVATE cxx_std_20)
target_link_libraries(amv_consumer PRIVATE amv::amv)
