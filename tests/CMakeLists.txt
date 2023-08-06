cmake_minimum_required(VERSION 3.20)

add_executable(main "main.cpp")

target_link_libraries(main PRIVATE shared_pointer)

target_compile_options(main PRIVATE "-Wall" "-Wextra" "-Wpedantic")

target_compile_features(main PRIVATE cxx_std_17)
set_target_properties(main PROPERTIES CXX_EXTENSIONS OFF)
