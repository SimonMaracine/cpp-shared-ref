cmake_minimum_required(VERSION 3.20)

project("cpp-shared-ref" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

option(CPP_SHARED_REF_BUILD_TESTS "Turn this on to build test binaries" OFF)
option(CPP_SHARED_REF_ASAN "Turn this on to enable sanitizers in unit tests" OFF)

add_library(cpp_shared_ref INTERFACE
    "src/cpp_shared_ref/internal/control_block.hpp"
    "src/cpp_shared_ref/memory.hpp"
    "src/cpp_shared_ref/version.hpp"
)

target_include_directories(cpp_shared_ref INTERFACE "src")

if(CPP_SHARED_REF_BUILD_TESTS)
    add_subdirectory(tests)
endif()

message(STATUS "cpp-shared-ref: Building tests: ${CPP_SHARED_REF_BUILD_TESTS}")
message(STATUS "cpp-shared-ref: Sanitizers: ${CPP_SHARED_REF_ASAN}")
