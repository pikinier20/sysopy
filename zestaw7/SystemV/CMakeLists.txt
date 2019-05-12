project(zad7)

cmake_minimum_required(VERSION 2.8)

add_library(belt belt.c)

add_executable(loader loader.c)
add_executable(trucker trucker.c)
add_executable(workergen workergen.c)

target_link_libraries(loader belt)
target_link_libraries(trucker belt)
target_link_libraries(workergen belt)