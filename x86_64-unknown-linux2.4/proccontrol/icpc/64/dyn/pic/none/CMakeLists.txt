# CMakeLists for proccontrol/icpc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}     -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_CC}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_CC} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_CC}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}     -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"cxx"
	"intel_CC 64 dyn"
	MUTATEEintelCC64dyncxx)

IF (MUTATEEintelCC64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targdyn_icpc_64_pic_none ${SOURCE_LIST_28})


INSTALL (TARGETS
		pc_exec_targdyn_icpc_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_CC failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_CC not present")
ENDIF()
