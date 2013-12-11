# CMakeLists for proccontrol/pgcxx/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test  ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test ")
IF (NOT ${M_pg_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test  "
	"${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test "
	"cxx"
	MUTATEEpgcxx32dyn)

IF (MUTATEEpgcxx32dyn MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targ.dyn_pgcxx_32_none_none ${SOURCE_LIST_28})


INSTALL (TARGETS
		pc_exec_targ.dyn_pgcxx_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
