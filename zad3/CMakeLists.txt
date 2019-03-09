project(main)
cmake_minimum_required(VERSION 2.8)

option(Opti "Optimalization" O0)
option(Lib "Library linking" STATIC)
SET(OPTIMALIZATION_FLAG "-O0")
if(Opti STREQUAL Os)
	SET(OPTIMALIZATION_FLAG "-Os")
endif(Opti STREQUAL Os)

if(Opti STREQUAL O3)
	SET(OPTIMALIZATION_FLAG "-O3")
endif(Opti STREQUAL O3)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c11 ${OPTIMALIZATION_FLAG}")
add_executable(zad2 main.c)

if(Lib STREQUAL STATIC)
	add_library(mylib STATIC mylib.c)
elseif(Lib STREQUAL SHARED)
	add_library(mylib SHARED mylib.c)
else()
	add_library(mylib STATIC mylib.c)
endif(Lib STREQUAL STATIC)



target_link_libraries(zad2 mylib)
