# CMakeLists for dyninst/bgxlc++/64/stat/none/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_mpi_xlcxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_mpi_xlcxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlcxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   "
	"${MUTATEE_LDFLAGS_NATIVE}  "
	"CXX"
	"mpi_xlcxx 64 stat"
	MUTATEEmpixlcxx64statCXX)

IF (MUTATEEmpixlcxx64statCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_cxx_group_test.stat_bgxlc++_64_none_none ${SOURCE_LIST_46})
set_source_files_properties(${SOURCE_LIST_46} PROPERTIES LANGUAGE CXX)
set_target_properties(dyninst_cxx_group_test.stat_bgxlc++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (dyninst_group_test.stat_bgxlc++_64_none_none ${SOURCE_LIST_29})
set_source_files_properties(${SOURCE_LIST_29} PROPERTIES LANGUAGE CXX)
set_target_properties(dyninst_group_test.stat_bgxlc++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (dyninst_group_test.stat_bgxlc++_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_bgxlc++_64_none_none ${SOURCE_LIST_39})
set_source_files_properties(${SOURCE_LIST_39} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_14.stat_bgxlc++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_29.stat_bgxlc++_64_none_none ${SOURCE_LIST_9})
set_source_files_properties(${SOURCE_LIST_9} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_29.stat_bgxlc++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		dyninst_cxx_group_test.stat_bgxlc++_64_none_none
		dyninst_group_test.stat_bgxlc++_64_none_none
		test1_14.stat_bgxlc++_64_none_none
		test1_29.stat_bgxlc++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_xlcxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_xlcxx not present")
ENDIF()
