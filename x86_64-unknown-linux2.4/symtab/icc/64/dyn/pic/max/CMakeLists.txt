# CMakeLists for symtab/icc/64/dyn/pic/max
set (CMAKE_C_FLAGS "${TEST_DEFINES} -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}   -O3 -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILE_OBJECT "${M_intel_cc} -c <FLAGS> -o <OBJECT> -c <SOURCE>")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_icc_64_pic_max ${SOURCE_LIST_50})
target_link_libraries (symtab_group_test.dyn_icc_64_pic_max testA)


INSTALL (TARGETS
		symtab_group_test.dyn_icc_64_pic_max
	DESTINATION testsuite)
