project(main)
cmake_minimum_required(VERSION 2.8)
add_compile_options("-Wall")
add_library(lib STATIC lib.c)

add_executable(main main.c)

target_link_libraries(main lib)

