# CMakeLists for dyninst/g++/32/stat/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}  -m32 -Di386_unknown_linux2_4 -Dm32_test  -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -static")
include (${PROJECT_SRC_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_group_test.stat_g++_32_pic_none ${SOURCE_LIST_4})
target_link_libraries (dyninst_group_test.stat_g++_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_g++_32_pic_none ${SOURCE_LIST_53})
add_executable (test1_29.stat_g++_32_pic_none ${SOURCE_LIST_40})


INSTALL (TARGETS
		dyninst_group_test.stat_g++_32_pic_none
		test1_14.stat_g++_32_pic_none
		test1_29.stat_g++_32_pic_none
	DESTINATION ${INSTALL_DIR})
