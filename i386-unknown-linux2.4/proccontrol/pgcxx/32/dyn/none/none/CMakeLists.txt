# CMakeLists for proccontrol/pgcxx/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_pg_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}    "
	"${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"cxx"
	"pg_cxx 32 dyn"
	MUTATEEpgcxx32dyncxx)

IF (MUTATEEpgcxx32dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targdyn_pgcxx_32_none_none ${SOURCE_LIST_11})


INSTALL (TARGETS
		pc_exec_targdyn_pgcxx_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for pg_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler pg_cxx not present")
ENDIF()
