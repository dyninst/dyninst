# CMakeLists for ppc64_bgq_ion/dyninst/bgxlc/64/stat/none/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}    -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_bg_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  ")
include (../../../../../../srclists.cmake)
add_executable (dyninst_group_test.stat_bgxlc_64_none_none ${SOURCE_LIST_26})
target_link_libraries (dyninst_group_test.stat_bgxlc_64_none_none dl)
add_executable (test1_14.stat_bgxlc_64_none_none ${SOURCE_LIST_38})
add_executable (test1_29.stat_bgxlc_64_none_none ${SOURCE_LIST_9})


INSTALL (TARGETS
		dyninst_group_test.stat_bgxlc_64_none_none
		test1_14.stat_bgxlc_64_none_none
		test1_29.stat_bgxlc_64_none_none
	DESTINATION testsuite)
