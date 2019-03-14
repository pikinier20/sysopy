project(main)
cmake_minimum_required(VERSION 2.8)
add_compile_options("-Wall")
add_library(file_sys STATIC file_sys.c)
add_library(file_lib STATIC file_lib.c)

add_executable(main main.c)

target_link_libraries(main file_sys)
target_link_libraries(main file_lib)

