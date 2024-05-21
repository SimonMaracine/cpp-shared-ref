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

set_compile_options_and_features(test_unit)

if(UNIX)
    target_compile_options(test_unit PRIVATE "-fno-elide-constructors")
endif()

if(UNIX AND CPP_SHARED_REF_ASAN)
    target_compile_options(test_unit PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-g" "-fno-omit-frame-pointer")
    target_link_options(test_unit PRIVATE "-fsanitize=address" "-fsanitize=undefined")
endif()
