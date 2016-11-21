# CMakeLists for symtab/gcc/32/dyn/none/high
set (CMAKE_C_FLAGS "${TEST_DEFINES} -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -m32 -Di386_unknown_linux2_4 -Dm32_test -O2 ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILE_OBJECT "${M_gnu_cc} -c <FLAGS> -o <OBJECT> -c <SOURCE>")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -Wl,-export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_gcc_32_none_high ${SOURCE_LIST_50})
target_link_libraries (symtab_group_test.dyn_gcc_32_none_high testA)


INSTALL (TARGETS
		symtab_group_test.dyn_gcc_32_none_high
	DESTINATION testsuite)
