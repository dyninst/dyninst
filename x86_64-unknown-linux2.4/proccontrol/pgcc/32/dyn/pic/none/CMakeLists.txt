# CMakeLists for proccontrol/pgcc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test  -KPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test ")
IF (NOT ${M_pg_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test  -KPIC"
	"${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test "
	"c"
	"pg_cc 32 dyn"
	MUTATEEpgcc32dync)

IF (MUTATEEpgcc32dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targdyn_pgcc_32_pic_none ${SOURCE_LIST_28})


INSTALL (TARGETS
		pc_exec_targdyn_pgcc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for pg_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler pg_cc not present")
ENDIF()
