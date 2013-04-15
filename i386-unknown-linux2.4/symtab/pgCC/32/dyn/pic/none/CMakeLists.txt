# CMakeLists for i386-unknown-linux2.4/symtab/pgCC/32/dyn/pic/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}    -KPIC -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_pgCC_32_pic_none ${SOURCE_LIST_50})
target_link_libraries (symtab_group_test.dyn_pgCC_32_pic_none testA_m32)


INSTALL (TARGETS
		symtab_group_test.dyn_pgCC_32_pic_none
	DESTINATION testsuite)
