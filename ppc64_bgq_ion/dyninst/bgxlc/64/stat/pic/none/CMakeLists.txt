# CMakeLists for dyninst/bgxlc/64/stat/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}   -qpic")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_xlc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_mpi_xlc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}   -qpic"
	"${MUTATEE_LDFLAGS_NATIVE}  "
	"c"
	MUTATEEmpixlc64stat)

IF (MUTATEEmpixlc64stat MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_group_test.stat_bgxlc_64_pic_none ${SOURCE_LIST_29})
target_link_libraries (dyninst_group_test.stat_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_bgxlc_64_pic_none ${SOURCE_LIST_39})
add_executable (test1_29.stat_bgxlc_64_pic_none ${SOURCE_LIST_9})


INSTALL (TARGETS
		dyninst_group_test.stat_bgxlc_64_pic_none
		test1_14.stat_bgxlc_64_pic_none
		test1_29.stat_bgxlc_64_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
