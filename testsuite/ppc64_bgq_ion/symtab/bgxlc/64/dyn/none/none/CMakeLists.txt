# CMakeLists for ppc64_bgq_ion/symtab/bgxlc/64/dyn/none/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}    -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_bg_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
include (../../../../../../srclists.cmake)
add_executable (symtab_group_test.dyn_bgxlc_64_none_none ${SOURCE_LIST_7})
target_link_libraries (symtab_group_test.dyn_bgxlc_64_none_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_bgxlc_64_none_none
	DESTINATION testsuite)
