# CMakeLists for dyninst/bgq_g++/64/stat/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_GNU}  -static")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_mpi_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -static"
	"cxx"
	"mpi_cxx 64 stat"
	MUTATEEmpicxx64statcxx)

IF (MUTATEEmpicxx64statcxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_cxx_group_test.stat_bgq_g++_64_none_none ${SOURCE_LIST_46})
add_executable (dyninst_group_test.stat_bgq_g++_64_none_none ${SOURCE_LIST_29})
target_link_libraries (dyninst_group_test.stat_bgq_g++_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_bgq_g++_64_none_none ${SOURCE_LIST_39})
add_executable (test1_29.stat_bgq_g++_64_none_none ${SOURCE_LIST_9})


INSTALL (TARGETS
		dyninst_cxx_group_test.stat_bgq_g++_64_none_none
		dyninst_group_test.stat_bgq_g++_64_none_none
		test1_14.stat_bgq_g++_64_none_none
		test1_29.stat_bgq_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
