# CMakeLists for dyninst/gcc/32/stat/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_32_none_none ${SOURCE_LIST_69})
target_link_libraries (dyninst_group_test.stat_gcc_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_gcc_32_none_none ${SOURCE_LIST_70})
add_executable (test1_29.stat_gcc_32_none_none ${SOURCE_LIST_58})
add_executable (test1_35.stat_gcc_32_none_none ${SOURCE_LIST_91})


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_32_none_none
		test1_14.stat_gcc_32_none_none
		test1_29.stat_gcc_32_none_none
		test1_35.stat_gcc_32_none_none
	DESTINATION ${INSTALL_DIR})
