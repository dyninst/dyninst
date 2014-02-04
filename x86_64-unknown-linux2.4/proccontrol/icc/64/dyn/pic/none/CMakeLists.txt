# CMakeLists for proccontrol/icc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"C"
	"intel_cc 64 dyn"
	MUTATEEintelcc64dynC)

IF (MUTATEEintelcc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targ.dyn_icc_64_pic_none ${SOURCE_LIST_28})
set_source_files_properties(${SOURCE_LIST_28} PROPERTIES LANGUAGE C)
set_target_properties(pc_exec_targ.dyn_icc_64_pic_none PROPERTIES LINKER_LANGUAGE C)


INSTALL (TARGETS
		pc_exec_targ.dyn_icc_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_cc not present")
ENDIF()
