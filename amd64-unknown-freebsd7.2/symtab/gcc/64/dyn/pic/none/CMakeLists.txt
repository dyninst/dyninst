# CMakeLists for amd64-unknown-freebsd7.2/symtab/gcc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    -fPIC -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_gcc_64_pic_none ${SOURCE_LIST_19})
target_link_libraries (symtab_group_test.dyn_gcc_64_pic_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_gcc_64_pic_none
	DESTINATION testsuite)
