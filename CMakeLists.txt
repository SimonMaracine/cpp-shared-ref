cmake_minimum_required(VERSION 3.20)

option(SHARED_POINTER_BUILD_TESTS "Turn this on to build test binaries" OFF)

project("shared-pointer" LANGUAGES CXX)

add_library(shared_pointer INTERFACE
    "src/shared_pointer/parts/internal/control_block.hpp"
    "src/shared_pointer/parts/shared_ref.hpp"
    "src/shared_pointer/parts/version.hpp"
    "src/shared_pointer/parts/weak_ref.hpp"
    "src/shared_pointer/shared_pointer.hpp"
)

target_include_directories(shared_pointer INTERFACE "src")

if(SHARED_POINTER_BUILD_TESTS)
    add_subdirectory(tests)
endif()

message(STATUS "shared_pointer: Building tests: ${SHARED_POINTER_BUILD_TESTS}")
