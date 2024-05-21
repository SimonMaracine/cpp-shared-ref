cmake_minimum_required(VERSION 3.20)

add_executable(test_speed "main.cpp")

target_link_libraries(test_speed PRIVATE cpp_shared_ref)

set_compile_options_and_features(test_speed)

if(UNIX)
    target_compile_options(test_speed PRIVATE "-O2")
endif()
