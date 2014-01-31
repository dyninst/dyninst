# CMakeLists for proccontrol/icc/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_cc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"c"
	"intel_cc 32 dyn"
	MUTATEEintelcc32dync)

IF (MUTATEEintelcc32dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targdyn_icc_32_none_none ${SOURCE_LIST_11})


INSTALL (TARGETS
		pc_exec_targdyn_icc_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_cc not present")
ENDIF()
