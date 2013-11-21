# CMakeLists for dyninst/gcc/32/stat/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
include (${PROJECT_SOURCE_DIR}/ppc32_linux/srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_32_pic_none ${SOURCE_LIST_58})
target_link_libraries (dyninst_group_test.stat_gcc_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_gcc_32_pic_none ${SOURCE_LIST_41})
add_executable (test1_29.stat_gcc_32_pic_none ${SOURCE_LIST_4})


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_32_pic_none
		test1_14.stat_gcc_32_pic_none
		test1_29.stat_gcc_32_pic_none
	DESTINATION ${INSTALL_DIR})
