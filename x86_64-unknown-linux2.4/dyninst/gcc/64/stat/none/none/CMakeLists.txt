# CMakeLists for dyninst/gcc/64/stat/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -static"
	"c"
	MUTATEEgnucc64stat)

IF (MUTATEEgnucc64stat MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_64_none_none ${SOURCE_LIST_48})
target_link_libraries (dyninst_group_test.stat_gcc_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_gcc_64_none_none ${SOURCE_LIST_53})
add_executable (test1_29.stat_gcc_64_none_none ${SOURCE_LIST_4})
add_executable (test1_35.stat_gcc_64_none_none ${SOURCE_LIST_44})


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_64_none_none
		test1_14.stat_gcc_64_none_none
		test1_29.stat_gcc_64_none_none
		test1_35.stat_gcc_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
