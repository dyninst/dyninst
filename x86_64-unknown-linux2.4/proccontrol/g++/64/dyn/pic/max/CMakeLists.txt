# CMakeLists for proccontrol/g++/64/dyn/pic/max
set (CMAKE_C_FLAGS "${TEST_DEFINES} -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}   -O3 -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILE_OBJECT "${M_gnu_cxx} -c <FLAGS> -o <OBJECT> -c <SOURCE>")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (pc_exec_targ.dyn_g++_64_pic_max ${SOURCE_LIST_27})


INSTALL (TARGETS
		pc_exec_targ.dyn_g++_64_pic_max
	DESTINATION testsuite)
