# CMakeLists for dyninst/gcc/32/stat/pic/high
set (CMAKE_C_FLAGS "${TEST_DEFINES} -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -m32 -Di386_unknown_linux2_4 -Dm32_test -O2 -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILE_OBJECT "${M_gnu_cc} -c <FLAGS> -o <OBJECT> -c <SOURCE>")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -static")
include (../../../../../../srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_32_pic_high ${SOURCE_LIST_36})
target_link_libraries (dyninst_group_test.stat_gcc_32_pic_high dl)
add_executable (test1_14.stat_gcc_32_pic_high ${SOURCE_LIST_53})
add_executable (test1_29.stat_gcc_32_pic_high ${SOURCE_LIST_4})


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_32_pic_high
		test1_14.stat_gcc_32_pic_high
		test1_29.stat_gcc_32_pic_high
	DESTINATION testsuite)
