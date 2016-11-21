# CMakeLists for dyninst/gcc/64/stat/pic/low
set (CMAKE_C_FLAGS "${TEST_DEFINES} -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}   -O1 -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILE_OBJECT "${M_gnu_cc} -c <FLAGS> -o <OBJECT> -c <SOURCE>")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
include (../../../../../../srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_64_pic_low ${SOURCE_LIST_36})
target_link_libraries (dyninst_group_test.stat_gcc_64_pic_low dl)
add_executable (test1_14.stat_gcc_64_pic_low ${SOURCE_LIST_53})
add_executable (test1_29.stat_gcc_64_pic_low ${SOURCE_LIST_4})
add_executable (test1_35.stat_gcc_64_pic_low ${SOURCE_LIST_44})


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_64_pic_low
		test1_14.stat_gcc_64_pic_low
		test1_29.stat_gcc_64_pic_low
		test1_35.stat_gcc_64_pic_low
	DESTINATION testsuite)
