# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sammael/study/bit_bucket/Project/MainProject

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sammael/study/bit_bucket/Project/MainProject

# Include any dependencies generated for this target.
include dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/depend.make

# Include the progress variables for this target.
include dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/progress.make

# Include the compile flags for this target's objects.
include dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/flags.make

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o: dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/flags.make
dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o: dependencies/include/TinyGLTF/impl.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sammael/study/bit_bucket/Project/MainProject/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o"
	cd /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/TinyGLTF.dir/impl.cpp.o -c /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF/impl.cpp

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TinyGLTF.dir/impl.cpp.i"
	cd /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF/impl.cpp > CMakeFiles/TinyGLTF.dir/impl.cpp.i

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TinyGLTF.dir/impl.cpp.s"
	cd /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF/impl.cpp -o CMakeFiles/TinyGLTF.dir/impl.cpp.s

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.requires:

.PHONY : dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.requires

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.provides: dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.requires
	$(MAKE) -f dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/build.make dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.provides.build
.PHONY : dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.provides

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.provides.build: dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o


# Object files for target TinyGLTF
TinyGLTF_OBJECTS = \
"CMakeFiles/TinyGLTF.dir/impl.cpp.o"

# External object files for target TinyGLTF
TinyGLTF_EXTERNAL_OBJECTS =

dependencies/include/TinyGLTF/libTinyGLTF.a: dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o
dependencies/include/TinyGLTF/libTinyGLTF.a: dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/build.make
dependencies/include/TinyGLTF/libTinyGLTF.a: dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sammael/study/bit_bucket/Project/MainProject/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libTinyGLTF.a"
	cd /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF && $(CMAKE_COMMAND) -P CMakeFiles/TinyGLTF.dir/cmake_clean_target.cmake
	cd /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TinyGLTF.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/build: dependencies/include/TinyGLTF/libTinyGLTF.a

.PHONY : dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/build

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/requires: dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/impl.cpp.o.requires

.PHONY : dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/requires

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/clean:
	cd /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF && $(CMAKE_COMMAND) -P CMakeFiles/TinyGLTF.dir/cmake_clean.cmake
.PHONY : dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/clean

dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/depend:
	cd /home/sammael/study/bit_bucket/Project/MainProject && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sammael/study/bit_bucket/Project/MainProject /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF /home/sammael/study/bit_bucket/Project/MainProject /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF /home/sammael/study/bit_bucket/Project/MainProject/dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dependencies/include/TinyGLTF/CMakeFiles/TinyGLTF.dir/depend

