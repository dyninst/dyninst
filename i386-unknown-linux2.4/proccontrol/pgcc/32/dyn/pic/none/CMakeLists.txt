# CMakeLists for i386-unknown-linux2.4/proccontrol/pgcc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_NATIVE}    -KPIC -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
include (../../../../../../srclists.cmake)
add_executable (pc_exec_targ.dyn_pgcc_32_pic_none ${SOURCE_LIST_11})


INSTALL (TARGETS
		pc_exec_targ.dyn_pgcc_32_pic_none
	DESTINATION testsuite)
