cmake_minimum_required(VERSION 3.20)

add_executable(test_speed
    "main.cpp"
)

target_link_libraries(test_speed PRIVATE cpp_shared_ref)

if(UNIX)
    target_compile_options(test_speed PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-O2")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(test_speed PRIVATE "/W4")
else()
    message(WARNING "Warnings are not enabled")
endif()

target_compile_features(test_speed PRIVATE cxx_std_17)
set_target_properties(test_speed PROPERTIES CXX_EXTENSIONS OFF)
