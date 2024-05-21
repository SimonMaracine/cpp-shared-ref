cmake_minimum_required(VERSION 3.20)

# My version

add_executable(test_shared_ref "shared_ref.cpp")

target_link_libraries(test_shared_ref PRIVATE cpp_shared_ref)

set_compile_options_and_features(test_shared_ref)

if(UNIX)
    target_compile_options(test_shared_ref PRIVATE "-g" "-fno-inline")
endif()

# STL version

add_executable(test_shared_ptr "shared_ptr.cpp")

set_compile_options_and_features(test_shared_ptr)

if(UNIX)
    target_compile_options(test_shared_ptr PRIVATE "-g" "-fno-inline")
endif()
