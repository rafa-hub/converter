# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

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
CMAKE_COMMAND = /opt/cmake-3.8.2-Linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.8.2-Linux-x86_64/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /var/tmp/Pruebas_Compilacion/copt

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /var/tmp/Pruebas_Compilacion/copt/build

# Include any dependencies generated for this target.
include copt/CMakeFiles/bbmcsp.dir/depend.make

# Include the progress variables for this target.
include copt/CMakeFiles/bbmcsp.dir/progress.make

# Include the compile flags for this target's objects.
include copt/CMakeFiles/bbmcsp.dir/flags.make

copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o: copt/CMakeFiles/bbmcsp.dir/flags.make
copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o: ../copt/examples/bbmcsp.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/var/tmp/Pruebas_Compilacion/copt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o"
	cd /var/tmp/Pruebas_Compilacion/copt/build/copt && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o -c /var/tmp/Pruebas_Compilacion/copt/copt/examples/bbmcsp.cpp

copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.i"
	cd /var/tmp/Pruebas_Compilacion/copt/build/copt && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /var/tmp/Pruebas_Compilacion/copt/copt/examples/bbmcsp.cpp > CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.i

copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.s"
	cd /var/tmp/Pruebas_Compilacion/copt/build/copt && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /var/tmp/Pruebas_Compilacion/copt/copt/examples/bbmcsp.cpp -o CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.s

copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.requires:

.PHONY : copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.requires

copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.provides: copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.requires
	$(MAKE) -f copt/CMakeFiles/bbmcsp.dir/build.make copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.provides.build
.PHONY : copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.provides

copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.provides.build: copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o


# Object files for target bbmcsp
bbmcsp_OBJECTS = \
"CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o"

# External object files for target bbmcsp
bbmcsp_EXTERNAL_OBJECTS =

copt/bbmcsp: copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o
copt/bbmcsp: copt/CMakeFiles/bbmcsp.dir/build.make
copt/bbmcsp: copt/libcopt.a
copt/bbmcsp: graph/libgraph.a
copt/bbmcsp: bitscan/libbitscan.a
copt/bbmcsp: utils/libutils.a
copt/bbmcsp: copt/CMakeFiles/bbmcsp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/var/tmp/Pruebas_Compilacion/copt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bbmcsp"
	cd /var/tmp/Pruebas_Compilacion/copt/build/copt && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bbmcsp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
copt/CMakeFiles/bbmcsp.dir/build: copt/bbmcsp

.PHONY : copt/CMakeFiles/bbmcsp.dir/build

copt/CMakeFiles/bbmcsp.dir/requires: copt/CMakeFiles/bbmcsp.dir/examples/bbmcsp.cpp.o.requires

.PHONY : copt/CMakeFiles/bbmcsp.dir/requires

copt/CMakeFiles/bbmcsp.dir/clean:
	cd /var/tmp/Pruebas_Compilacion/copt/build/copt && $(CMAKE_COMMAND) -P CMakeFiles/bbmcsp.dir/cmake_clean.cmake
.PHONY : copt/CMakeFiles/bbmcsp.dir/clean

copt/CMakeFiles/bbmcsp.dir/depend:
	cd /var/tmp/Pruebas_Compilacion/copt/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /var/tmp/Pruebas_Compilacion/copt /var/tmp/Pruebas_Compilacion/copt/copt /var/tmp/Pruebas_Compilacion/copt/build /var/tmp/Pruebas_Compilacion/copt/build/copt /var/tmp/Pruebas_Compilacion/copt/build/copt/CMakeFiles/bbmcsp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : copt/CMakeFiles/bbmcsp.dir/depend

