# CMakeLists for dyninst/bgxlc++/64/stat/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_xlcxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_mpi_xlcxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlcxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   "
	"${MUTATEE_LDFLAGS_NATIVE}  "
	"cxx"
	"mpi_xlcxx 64 stat"
	MUTATEEmpixlcxx64statcxx)

IF (MUTATEEmpixlcxx64statcxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_cxx_group_teststat_bgxlc++_64_none_none ${SOURCE_LIST_46})
add_executable (dyninst_group_teststat_bgxlc++_64_none_none ${SOURCE_LIST_29})
target_link_libraries (dyninst_group_teststat_bgxlc++_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14stat_bgxlc++_64_none_none ${SOURCE_LIST_39})
add_executable (test1_29stat_bgxlc++_64_none_none ${SOURCE_LIST_9})


INSTALL (TARGETS
		dyninst_cxx_group_teststat_bgxlc++_64_none_none
		dyninst_group_teststat_bgxlc++_64_none_none
		test1_14stat_bgxlc++_64_none_none
		test1_29stat_bgxlc++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_xlcxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_xlcxx not present")
ENDIF()
