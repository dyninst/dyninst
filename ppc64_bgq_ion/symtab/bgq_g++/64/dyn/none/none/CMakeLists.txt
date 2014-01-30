# CMakeLists for symtab/bgq_g++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
IF (NOT ${M_mpi_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic"
	"cxx"
	"mpi_cxx 64 dyn"
	MUTATEEmpicxx64dyncxx)

IF (MUTATEEmpicxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (symtab_group_testdyn_bgq_g++_64_none_none ${SOURCE_LIST_19})
target_link_libraries (symtab_group_testdyn_bgq_g++_64_none_none testA)


INSTALL (TARGETS
		symtab_group_testdyn_bgq_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_cxx not present")
ENDIF()
