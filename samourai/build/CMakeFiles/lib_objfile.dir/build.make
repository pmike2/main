# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/home/git_dir/main/samourai

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/home/git_dir/main/samourai/build

# Include any dependencies generated for this target.
include CMakeFiles/lib_objfile.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lib_objfile.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lib_objfile.dir/flags.make

CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.o: CMakeFiles/lib_objfile.dir/flags.make
CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.o: /Users/home/git_dir/main/libs/objfile.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/home/git_dir/main/samourai/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.o -c /Users/home/git_dir/main/libs/objfile.cpp

CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/home/git_dir/main/libs/objfile.cpp > CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.i

CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/home/git_dir/main/libs/objfile.cpp -o CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.s

# Object files for target lib_objfile
lib_objfile_OBJECTS = \
"CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.o"

# External object files for target lib_objfile
lib_objfile_EXTERNAL_OBJECTS =

liblib_objfile.a: CMakeFiles/lib_objfile.dir/Users/home/git_dir/main/libs/objfile.cpp.o
liblib_objfile.a: CMakeFiles/lib_objfile.dir/build.make
liblib_objfile.a: CMakeFiles/lib_objfile.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/home/git_dir/main/samourai/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library liblib_objfile.a"
	$(CMAKE_COMMAND) -P CMakeFiles/lib_objfile.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lib_objfile.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lib_objfile.dir/build: liblib_objfile.a

.PHONY : CMakeFiles/lib_objfile.dir/build

CMakeFiles/lib_objfile.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lib_objfile.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lib_objfile.dir/clean

CMakeFiles/lib_objfile.dir/depend:
	cd /Users/home/git_dir/main/samourai/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/home/git_dir/main/samourai /Users/home/git_dir/main/samourai /Users/home/git_dir/main/samourai/build /Users/home/git_dir/main/samourai/build /Users/home/git_dir/main/samourai/build/CMakeFiles/lib_objfile.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lib_objfile.dir/depend
