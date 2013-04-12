# CMakeLists for symtab/pgCC/64/dyn/pic/high
set (CMAKE_C_FLAGS "${TEST_DEFINES} -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}   -O2 -KPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILE_OBJECT "${M_pg_cxx} -c <FLAGS> -o <OBJECT> -c <SOURCE>")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CXXFLAGS_NATIVE) ${MUTATEE_LDFLAGS_NATIVE}  ")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_pgCC_64_pic_high ${SOURCE_LIST_50})
target_link_libraries (symtab_group_test.dyn_pgCC_64_pic_high testA)


INSTALL (TARGETS
		symtab_group_test.dyn_pgCC_64_pic_high
	DESTINATION testsuite)
