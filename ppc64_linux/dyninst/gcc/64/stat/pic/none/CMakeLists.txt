# CMakeLists for ppc64_linux/dyninst/gcc/64/stat/pic/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -m64  -fPIC -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m64 -static")
include (../../../../../../srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_64_pic_none ${SOURCE_LIST_10})
target_link_libraries (dyninst_group_test.stat_gcc_64_pic_none dl)
add_executable (test1_14.stat_gcc_64_pic_none ${SOURCE_LIST_41})
add_executable (test1_29.stat_gcc_64_pic_none ${SOURCE_LIST_4})


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_64_pic_none
		test1_14.stat_gcc_64_pic_none
		test1_29.stat_gcc_64_pic_none
	DESTINATION testsuite)
