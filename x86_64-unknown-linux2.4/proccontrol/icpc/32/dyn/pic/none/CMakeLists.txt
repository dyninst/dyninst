# CMakeLists for proccontrol/icpc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}   -Di386_unknown_linux2_4 -Dm32_test  -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_CC_32}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic")
IF (NOT ${M_intel_CC_32} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_CC_32}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}   -Di386_unknown_linux2_4 -Dm32_test  -fPIC"
	"${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic"
	"cxx"
	"intel_CC_32 32 dyn"
	MUTATEEintelCC3232dyncxx)

IF (MUTATEEintelCC3232dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targdyn_icpc_32_pic_none ${SOURCE_LIST_28})


INSTALL (TARGETS
		pc_exec_targdyn_icpc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_CC_32 failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_CC_32 not present")
ENDIF()
