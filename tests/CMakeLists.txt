cmake_minimum_required(VERSION 3.20)

project(test)

add_executable(main main.cpp)

target_compile_options(main PRIVATE -Wall -Wextra -Wno-parentheses)

target_compile_features(main PRIVATE cxx_std_17)
set_target_properties(main PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(main PRIVATE "../src")
