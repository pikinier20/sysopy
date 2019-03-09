project(main)
cmake_minimum_required(VERSION 2.8)
enable_testing()

option(Opti "Optimalization" O0)
SET(OPTIMALIZATION_FLAG "-O0")
if(Opti STREQUAL Os)
	SET(OPTIMALIZATION_FLAG "-Os")
endif(Opti STREQUAL Os)

if(Opti STREQUAL O3)
	SET(OPTIMALIZATION_FLAG "-O3")
endif(Opti STREQUAL O3)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c11 ${OPTIMALIZATION_FLAG}")
SET(CMAKE_STATIC_LIBRARY_PREFIX "")
SET(CMAKE_SHARED_LIBRARY_PREFIX "")
SET(CTEST_OUTPUT_ON_FAILURE TRUE)


	add_library(staticlib STATIC mylib.c)
	add_executable(zad2static main.c)
	target_link_libraries(zad2static staticlib)

	add_library(sharedlib SHARED mylib.c)
	add_executable(zad2shared main.c)
	target_link_libraries(zad2shared sharedlib)

	add_executable(zad2dynamic main_dynamic.c)
	target_link_libraries(zad2dynamic sharedlib)
	target_link_libraries(zad2dynamic dl)

add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Static)
add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Hard test)

add_custom_command( TARGET zad2static POST_BUILD COMMAND ./zad2static create_table 10 search_directory / main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Medium test)

add_custom_command( TARGET zad2static POST_BUILD COMMAND ./zad2static create_table 10 search_directory home main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Easy test)

add_custom_command( TARGET zad2static POST_BUILD COMMAND ./zad2static create_table 10 search_directory . main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo )
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo )
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Shared)
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Hard test)

add_custom_command( TARGET zad2shared POST_BUILD COMMAND ./zad2shared create_table 10 search_directory / main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Medium test)

add_custom_command( TARGET zad2shared POST_BUILD COMMAND ./zad2shared create_table 10 search_directory home main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Easy test)

add_custom_command( TARGET zad2shared POST_BUILD COMMAND ./zad2shared create_table 10 search_directory . main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)


add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo )
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo )
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Dynamic)
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Hard test)

add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND ./zad2dynamic create_table 10 search_directory / main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Medium test)

add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND ./zad2dynamic create_table 10 search_directory home main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Easy test)

add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND ./zad2dynamic create_table 10 search_directory . main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)





