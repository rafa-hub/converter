# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
set(useful_headers_for_IDE ./simple_graph.h
./simple_ugraph.h
./kcore.h
./graph_gen.h
./algorithms/graph_sort.h
./algorithms/graph_map.h
)


#### SCRIPT to add LEDA to graph library(only Win32)#####
set(LEDA_ON OFF CACHE BOOL "config")
	
if(LEDA_ON)
add_definitions(-DLEDA_ON)				#my switch	
set(CMAKE_CXX_FLAGS "-DLEDA_DLL")			#leda switch

#set root depending on installation
set(LEDA_ROOT C:/Leda63/)

#external libraries
include_directories(${LEDA_ROOT}/incl)
link_directories(${LEDA_ROOT})

add_library (graph ./formats/mmio.cpp 
${useful_headers_for_IDE} 
./draw/ldraw.cpp
)

TARGET_LINK_LIBRARIES (graph debug leda_mdd.lib)
TARGET_LINK_LIBRARIES( graph optimized leda_md.lib)

else()

add_library (graph ./formats/mmio.cpp
${useful_headers_for_IDE}
)

endif()
###END LEDA ##########


# Make sure the compiler can find include files for our bitscan library
# when other libraries or executables link to bitscan
target_include_directories (graph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

option(graph_build_tests "Build graph tests." OFF)
if (graph_build_tests)

# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
add_executable (test_graph ./tests/gtest_main.cpp
#./tests/test_degree.cpp
#./tests/test_format.cpp
# ./tests/test_graph.cpp
#./tests/test_func.cpp
#./tests/test_graph_properties_in_folder.cpp
# ./tests/test_graph_sort.cpp
# ./tests/test_kcore.cpp
#./tests/test_kcore_in_folder.cpp
# ./tests/test_random_graph.cpp
#./tests/test_sparse_graph.cpp
# ./tests/test_fixed_graph.cpp
# ./tests/test_wgraph.cpp
 ./tests/test_graph_map.cpp
)

target_include_directories (test_graph PUBLIC ../googletest/include/)
# Link the executable to the Hello library. Since the Hello library has
# public include directories we will use those link directories when building
# helloDemo
target_link_libraries (test_graph LINK_PUBLIC graph bitscan utils gtest)

endif()

option(graph_build_examples "Build graph examples." OFF)
if (graph_build_examples)

add_executable ( gen_random ./examples/gen_random.cpp)
target_link_libraries (gen_random LINK_PUBLIC graph bitscan utils)
add_executable ( graph_formats ./examples/graph_formats.cpp)
target_link_libraries (graph_formats LINK_PUBLIC graph bitscan utils)
add_executable ( lb_kcore ./examples/lb_kcore.cpp)
target_link_libraries (lb_kcore LINK_PUBLIC graph bitscan utils)

if(LEDA_ON)
add_executable ( gwin_main ./examples/gwin_main.cpp)
target_link_libraries (gwin_main LINK_PUBLIC graph bitscan utils)
endif()

endif()

#file(GLOB_RECURSE DEPLOY_FILES_AND_DIRS "${PROJECT_SOURCE_DIR}/*.h")
#foreach(ITEM ${DEPLOY_FILES_AND_DIRS})
#	  file(RELATIVE_PATH dir ${PROJECT_SOURCE_DIR} ${ITEM})
#      get_filename_component( dir ${ITEM} DIRECTORY )
#      install( FILES ${ITEM} DESTINATION include/graph/${dir} )
#endforeach()

install(TARGETS graph DESTINATION lib)

