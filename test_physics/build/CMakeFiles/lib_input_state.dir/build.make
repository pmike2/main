# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

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
CMAKE_COMMAND = /Applications/ADD_SOFTS/DEV/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/ADD_SOFTS/DEV/CMake.app/Contents/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Volumes/malenge/dir_git/main/test_physics

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Volumes/malenge/dir_git/main/test_physics/build

# Include any dependencies generated for this target.
include CMakeFiles/lib_input_state.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lib_input_state.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lib_input_state.dir/flags.make

CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.o: CMakeFiles/lib_input_state.dir/flags.make
CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.o: /Volumes/malenge/dir_git/main/libs/input_state.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Volumes/malenge/dir_git/main/test_physics/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.o -c /Volumes/malenge/dir_git/main/libs/input_state.cpp

CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Volumes/malenge/dir_git/main/libs/input_state.cpp > CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.i

CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Volumes/malenge/dir_git/main/libs/input_state.cpp -o CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.s

# Object files for target lib_input_state
lib_input_state_OBJECTS = \
"CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.o"

# External object files for target lib_input_state
lib_input_state_EXTERNAL_OBJECTS =

liblib_input_state.a: CMakeFiles/lib_input_state.dir/Volumes/malenge/dir_git/main/libs/input_state.cpp.o
liblib_input_state.a: CMakeFiles/lib_input_state.dir/build.make
liblib_input_state.a: CMakeFiles/lib_input_state.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Volumes/malenge/dir_git/main/test_physics/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library liblib_input_state.a"
	$(CMAKE_COMMAND) -P CMakeFiles/lib_input_state.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lib_input_state.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lib_input_state.dir/build: liblib_input_state.a

.PHONY : CMakeFiles/lib_input_state.dir/build

CMakeFiles/lib_input_state.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lib_input_state.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lib_input_state.dir/clean

CMakeFiles/lib_input_state.dir/depend:
	cd /Volumes/malenge/dir_git/main/test_physics/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Volumes/malenge/dir_git/main/test_physics /Volumes/malenge/dir_git/main/test_physics /Volumes/malenge/dir_git/main/test_physics/build /Volumes/malenge/dir_git/main/test_physics/build /Volumes/malenge/dir_git/main/test_physics/build/CMakeFiles/lib_input_state.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lib_input_state.dir/depend
