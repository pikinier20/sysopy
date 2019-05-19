project(zad3)

cmake_minimum_required(VERSION 2.8)
add_compile_options("-Wall")
add_compile_options("-std=c11")

add_executable(main main.c)

target_link_libraries(main pthread m)