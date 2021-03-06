project(main)
cmake_minimum_required(VERSION 2.8)
enable_testing()

option(O3 "O3" OFF)
option(Os "Os" OFF)

if(Os)
	add_compile_options("-Os")
endif(Os)

if(O3)
	add_compile_options("-O3")
endif(O3)

SET(CMAKE_STATIC_LIBRARY_PREFIX "")
SET(CMAKE_SHARED_LIBRARY_PREFIX "")
SET(CTEST_OUTPUT_ON_FAILURE TRUE)
add_compile_options("-Wall")


	add_library(staticlib STATIC mylib.c)
	add_executable(zad2static main.c)
	target_link_libraries(zad2static staticlib)

	add_library(sharedlib SHARED mylib.c)
	add_executable(zad2shared main.c)
	target_link_libraries(zad2shared sharedlib)

	add_executable(zad2dynamic main_dynamic.c)
	target_link_libraries(zad2dynamic dl)
SET(EASY_TEST create_table 10 search_directory . main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)
SET(MEDIUM_TEST create_table 10 search_directory home main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)
SET(HARD_TEST create_table 10 search_directory / main.c temp save_search temp remove_block 0 save_search temp remove_block 0 save_search temp save_search temp remove_block 0 remove_block 1)

add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Static)
add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Hard test)
add_custom_command( TARGET zad2static POST_BUILD COMMAND ./zad2static ${HARD_TEST})
add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Medium test)
add_custom_command( TARGET zad2static POST_BUILD COMMAND ./zad2static ${MEDIUM_TEST})
add_custom_command( TARGET zad2static POST_BUILD COMMAND echo Easy test)
add_custom_command( TARGET zad2static POST_BUILD COMMAND ./zad2static  ${EASY_TEST})
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo )
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo )
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Shared)
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Hard test)
add_custom_command( TARGET zad2shared POST_BUILD COMMAND ./zad2shared ${HARD_TEST})
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Medium test)
add_custom_command( TARGET zad2shared POST_BUILD COMMAND ./zad2shared ${MEDIUM_TEST})
add_custom_command( TARGET zad2shared POST_BUILD COMMAND echo Easy test)
add_custom_command( TARGET zad2shared POST_BUILD COMMAND ./zad2shared ${EASY_TEST})
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo)
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo)
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Dynamic)
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Hard test)
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND ./zad2dynamic ${HARD_TEST})
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Medium test)
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND ./zad2dynamic ${MEDIUM_TEST})
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND echo Easy test)
add_custom_command( TARGET zad2dynamic POST_BUILD COMMAND ./zad2dynamic ${EASY_TEST})





