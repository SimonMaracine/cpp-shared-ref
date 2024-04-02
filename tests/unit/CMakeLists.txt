cmake_minimum_required(VERSION 3.20)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

add_subdirectory(extern/googletest)

add_executable(test
    "shared_ref.cpp"
    "types.hpp"
    "weak_ref.cpp"
)

target_link_libraries(test PRIVATE shared_ref GTest::gtest_main)

target_compile_options(test PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-fno-elide-constructors")

target_compile_features(test PRIVATE cxx_std_17)
set_target_properties(test PROPERTIES CXX_EXTENSIONS OFF)
