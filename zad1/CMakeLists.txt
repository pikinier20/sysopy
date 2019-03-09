project(main)
cmake_minimum_required(VERSION 2.8)
add_library(mystaticlib STATIC mylib.c)
add_library(mysharedlib SHARED mylib.c)

