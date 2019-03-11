project(main)
cmake_minimum_required(VERSION 2.8)
add_compile_options("-Wall")

add_executable(zad2 main.c)
add_library(mylib mylib.c)

target_link_libraries(zad2 mylib)
