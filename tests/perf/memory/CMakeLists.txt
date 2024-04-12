cmake_minimum_required(VERSION 3.20)

# My version

add_executable(test_shared_ref
    "shared_ref.cpp"
)

target_link_libraries(test_shared_ref PRIVATE cpp_shared_ref)

if(UNIX)
    target_compile_options(test_shared_ref PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-g" "-fno-inline")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(test_shared_ref PRIVATE "/W4")
else()
    message(WARNING "Warnings are not enabled")
endif()

target_compile_features(test_shared_ref PRIVATE cxx_std_17)
set_target_properties(test_shared_ref PROPERTIES CXX_EXTENSIONS OFF)

# STL version

add_executable(test_shared_ptr
    "shared_ptr.cpp"
)

if(UNIX)
    target_compile_options(test_shared_ptr PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-g" "-fno-inline")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(test_shared_ptr PRIVATE "/W4")
else()
    message(WARNING "Warnings are not enabled")
endif()

target_compile_features(test_shared_ptr PRIVATE cxx_std_17)
set_target_properties(test_shared_ptr PROPERTIES CXX_EXTENSIONS OFF)
