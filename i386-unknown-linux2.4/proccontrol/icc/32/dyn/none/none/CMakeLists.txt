# CMakeLists for proccontrol/icc/32/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"C"
	"intel_cc 32 dyn"
	MUTATEEintelcc32dynC)

IF (MUTATEEintelcc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targ.dyn_icc_32_none_none ${SOURCE_LIST_11})
set_source_files_properties(${SOURCE_LIST_11} PROPERTIES LANGUAGE C)
set_target_properties(pc_exec_targ.dyn_icc_32_none_none PROPERTIES LINKER_LANGUAGE C)


INSTALL (TARGETS
		pc_exec_targ.dyn_icc_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_cc not present")
ENDIF()
