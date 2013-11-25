# CMakeLists for dyninst/bgq_g++/64/stat/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_mpi_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -static"
	MUTATEEmpicxx64stat)

IF (MUTATEEmpicxx64stat)
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_cxx_group_test.stat_bgq_g++_64_pic_none ${SOURCE_LIST_45})
add_executable (dyninst_group_test.stat_bgq_g++_64_pic_none ${SOURCE_LIST_29})
target_link_libraries (dyninst_group_test.stat_bgq_g++_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_bgq_g++_64_pic_none ${SOURCE_LIST_39})
add_executable (test1_29.stat_bgq_g++_64_pic_none ${SOURCE_LIST_9})


INSTALL (TARGETS
		dyninst_cxx_group_test.stat_bgq_g++_64_pic_none
		dyninst_group_test.stat_bgq_g++_64_pic_none
		test1_14.stat_bgq_g++_64_pic_none
		test1_29.stat_bgq_g++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
