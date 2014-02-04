# CMakeLists for proccontrol/pgcc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}    -KPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_pg_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}    -KPIC"
	"${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"C"
	"pg_cc 32 dyn"
	MUTATEEpgcc32dynC)

IF (MUTATEEpgcc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targ.dyn_pgcc_32_pic_none ${SOURCE_LIST_11})
set_source_files_properties(${SOURCE_LIST_11} PROPERTIES LANGUAGE C)
set_target_properties(pc_exec_targ.dyn_pgcc_32_pic_none PROPERTIES LINKER_LANGUAGE C)


INSTALL (TARGETS
		pc_exec_targ.dyn_pgcc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for pg_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler pg_cc not present")
ENDIF()
