cmake_minimum_required(VERSION 3.10)
project(sockets C)

set(CMAKE_C_STANDARD 99)

add_executable(client client.c common.h)
add_executable(server server.c common.h)
target_link_libraries(server pthread)
target_link_libraries(client pthread)