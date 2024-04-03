cmake_minimum_required(VERSION 3.20)

# My version

add_executable(test_shared_ref
    "shared_ref.cpp"
)

target_link_libraries(test_shared_ref PRIVATE shared_ref)

target_compile_options(test_shared_ref PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-g" "-fno-inline")

target_compile_features(test_shared_ref PRIVATE cxx_std_17)
set_target_properties(test_shared_ref PROPERTIES CXX_EXTENSIONS OFF)

# STL version

add_executable(test_shared_ptr
    "shared_ptr.cpp"
)

target_compile_options(test_shared_ptr PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-g" "-fno-inline")

target_compile_features(test_shared_ptr PRIVATE cxx_std_17)
set_target_properties(test_shared_ptr PROPERTIES CXX_EXTENSIONS OFF)
