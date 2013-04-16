# CMakeLists for symtab/iCC/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS} -x c++ -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}     -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_CC}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_icpc_64_pic_none ${SOURCE_LIST_15})
target_link_libraries (symtab_group_test.dyn_icpc_64_pic_none testA)
add_executable (test_exception.dyn_icpc_64_pic_none ${SOURCE_LIST_69})


INSTALL (TARGETS
		symtab_group_test.dyn_icpc_64_pic_none
		test_exception.dyn_icpc_64_pic_none
	DESTINATION testsuite)
