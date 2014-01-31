# CMakeLists for proccontrol/icc/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -Di386_unknown_linux2_4 -Dm32_test  ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_cc_32}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic")
IF (NOT ${M_intel_cc_32} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_cc_32}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -Di386_unknown_linux2_4 -Dm32_test  "
	"${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic"
	"c"
	"intel_cc_32 32 dyn"
	MUTATEEintelcc3232dync)

IF (MUTATEEintelcc3232dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targdyn_icc_32_none_none ${SOURCE_LIST_28})


INSTALL (TARGETS
		pc_exec_targdyn_icc_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_cc_32 failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_cc_32 not present")
ENDIF()
