cmake_minimum_required(VERSION 3.20)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

add_subdirectory(extern/googletest)

add_executable(test_unit
    "enable_shared_from_this.cpp"
    "owner_less.cpp"
    "shared_ref.cpp"
    "types.hpp"
    "weak_ref.cpp"
)

target_link_libraries(test_unit PRIVATE cpp_shared_ref GTest::gtest_main)

if(UNIX)
    target_compile_options(test_unit PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wconversion" "-fno-elide-constructors")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(test_unit PRIVATE "/W4")
else()
    message(WARNING "Warnings are not enabled")
endif()

target_compile_features(test_unit PRIVATE cxx_std_17)
set_target_properties(test_unit PROPERTIES CXX_EXTENSIONS OFF)

if(UNIX AND CPP_SHARED_REF_ASAN)
    target_compile_options(test_unit PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-g" "-fno-omit-frame-pointer" "-libasan")
    target_link_options(test_unit PRIVATE "-fsanitize=address" "-fsanitize=undefined")
endif()
