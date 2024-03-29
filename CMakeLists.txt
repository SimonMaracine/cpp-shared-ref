cmake_minimum_required(VERSION 3.20)

project("cpp-shared-ref" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

option(CPP_SHARED_REF_BUILD_TESTS "Turn this on to build test binaries" OFF)

add_library(shared_ref INTERFACE
    "src/shared_ref/internal/control_block.hpp"
    "src/shared_ref/shared_ref.hpp"
    "src/shared_ref/version.hpp"
    "src/shared_ref/weak_ref.hpp"
)

target_include_directories(shared_ref INTERFACE "src")

if(CPP_SHARED_REF_BUILD_TESTS)
    add_subdirectory(tests)
endif()

message(STATUS "cpp-shared-ref: Building tests: ${CPP_SHARED_REF_BUILD_TESTS}")
