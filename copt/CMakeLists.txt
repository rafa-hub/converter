# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.


set(useful_headers_for_IDE ./clique/clique_types.h
./clique/clique_enum.h
./clique/clique_enum_sparse.h
./clique/clique_iter.h
./clique/clique_sat.h
./clique/clique_watched.h
./clique/clique.h
./clique/clique_infra.h
./clique/clique_infra_plus.h
./clique/clique_russian_doll.h
./clique/clique_russian_doll_plus.h
./clique/clique_weighted.h
./clique/clique_weighted_plus.h
./clique/clique_csp_plus.h
./clique/infra_tools.h
./clique/infra_tools_plus.h
./clique/infra_tools_plus_csp.h
./clique/infra_weighted_tools.h
./interfaces/interface_interdiction.h
./interfaces/interface_interdiction_plus.h
./interfaces/interface_partition.h
./clique/clique_all_max_sol.h
./init_color_sort.h
./init_csp.h
./clique/filter_csp_plus.h
./clique/heur/ub_weighted_clique.h
./clique/heur/super_weight.h
./clique/heur/cover_mwss.h
./setup.h
)

add_library (copt ./amts/mnts.cpp
./batch/benchmark_clique.cpp
./batch/benchmark_color.cpp
./init_color_sort.cpp
${useful_headers_for_IDE}
)

# Make sure the compiler can find include files for our bitscan library
# when other libraries or executables link to bitscan
#target_include_directories (copt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (copt PUBLIC ${CMAKE_SOURCE_DIR})

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

set(LIBRARY_NAME xcsp3parser)

set(LIB_HEADERS
        include/AttributeList.h
        include/UTF8String.h
        include/XCSP3Constants.h
        include/XCSP3Constraint.h
        include/XCSP3CoreParser.h
        include/XCSP3CoreCallbacks.h
        include/XCSP3Manager.h
        include/XCSP3Domain.h
        include/XCSP3Objective.h
        include/XCSP3utils.h
        include/XCSP3Variable.h
        include/XMLParser.h
	include/XCSP3Tree.h
        include/XCSP3TreeNode.h
        )

set(LIB_SOURCES
        src/UTF8String.cc
        src/XCSP3Code.cc
        src/XCSP3CoreParser.cc
        src/XCSP3Manager.cc
        src/XMLParser.cc
        src/XMLParserTags.cc
	src/XCSP3Tree.cc
        src/XCSP3TreeNode.cc
        )

set(APP_HEADERS
        )

set(APP_SOURCES
        csp_converter/XCSP3PrintCallbacks.h
        include/XCSP3TreeNode.h)

include_directories(
        ${PROJECT_SOURCE_DIR}/copt/include
	)

add_library(${LIBRARY_NAME} STATIC ${LIB_SOURCES} ${LIB_HEADERS})
target_link_libraries(${LIBRARY_NAME} ${LIBXML2_LIBRARIES})
set(APPLICATION_NAME converter)


find_package( Threads )
add_executable ( ${APPLICATION_NAME} converter/converter.cpp ${APP_SOURCES})
target_link_libraries (${APPLICATION_NAME} LINK_PUBLIC copt graph bitscan utils ${CMAKE_THREAD_LIBS_INIT})
target_include_directories (${APPLICATION_NAME} PUBLIC ../googletest/include/)
target_link_libraries(${APPLICATION_NAME} ${LIBRARY_NAME} ${LIBXML2_LIBRARIES})


install(TARGETS copt DESTINATION lib)



