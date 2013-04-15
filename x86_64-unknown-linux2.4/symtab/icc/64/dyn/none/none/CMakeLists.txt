# CMakeLists for x86_64-unknown-linux2.4/symtab/icc/64/dyn/none/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}     -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_icc_64_none_none ${SOURCE_LIST_15})
target_link_libraries (symtab_group_test.dyn_icc_64_none_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_icc_64_none_none
	DESTINATION testsuite)
