# CMakeLists for symtab/bgxlc++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_xlcxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
IF (NOT ${M_mpi_xlcxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlcxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   "
	"${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink"
	"cxx"
	MUTATEEmpixlcxx64dyn)

IF (MUTATEEmpixlcxx64dyn MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (symtab_group_test.dyn_bgxlc++_64_none_none ${SOURCE_LIST_19})
target_link_libraries (symtab_group_test.dyn_bgxlc++_64_none_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_bgxlc++_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
