# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/user/sysopy/zestaw2/zad1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/user/sysopy/zestaw2/zad1

# Include any dependencies generated for this target.
include CMakeFiles/file_lib.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/file_lib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/file_lib.dir/flags.make

CMakeFiles/file_lib.dir/file_lib.c.o: CMakeFiles/file_lib.dir/flags.make
CMakeFiles/file_lib.dir/file_lib.c.o: file_lib.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/sysopy/zestaw2/zad1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/file_lib.dir/file_lib.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/file_lib.dir/file_lib.c.o   -c /home/user/sysopy/zestaw2/zad1/file_lib.c

CMakeFiles/file_lib.dir/file_lib.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/file_lib.dir/file_lib.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/user/sysopy/zestaw2/zad1/file_lib.c > CMakeFiles/file_lib.dir/file_lib.c.i

CMakeFiles/file_lib.dir/file_lib.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/file_lib.dir/file_lib.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/user/sysopy/zestaw2/zad1/file_lib.c -o CMakeFiles/file_lib.dir/file_lib.c.s

# Object files for target file_lib
file_lib_OBJECTS = \
"CMakeFiles/file_lib.dir/file_lib.c.o"

# External object files for target file_lib
file_lib_EXTERNAL_OBJECTS =

libfile_lib.a: CMakeFiles/file_lib.dir/file_lib.c.o
libfile_lib.a: CMakeFiles/file_lib.dir/build.make
libfile_lib.a: CMakeFiles/file_lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/user/sysopy/zestaw2/zad1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libfile_lib.a"
	$(CMAKE_COMMAND) -P CMakeFiles/file_lib.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/file_lib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/file_lib.dir/build: libfile_lib.a

.PHONY : CMakeFiles/file_lib.dir/build

CMakeFiles/file_lib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/file_lib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/file_lib.dir/clean

CMakeFiles/file_lib.dir/depend:
	cd /home/user/sysopy/zestaw2/zad1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/user/sysopy/zestaw2/zad1 /home/user/sysopy/zestaw2/zad1 /home/user/sysopy/zestaw2/zad1 /home/user/sysopy/zestaw2/zad1 /home/user/sysopy/zestaw2/zad1/CMakeFiles/file_lib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/file_lib.dir/depend
