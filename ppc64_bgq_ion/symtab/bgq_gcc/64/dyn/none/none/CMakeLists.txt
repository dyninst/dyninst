# CMakeLists for symtab/bgq_gcc/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
IF (NOT ${M_mpi_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic"
	"c"
	"mpi_cc 64 dyn"
	MUTATEEmpicc64dync)

IF (MUTATEEmpicc64dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (symtab_group_testdyn_bgq_gcc_64_none_none ${SOURCE_LIST_19})
target_link_libraries (symtab_group_testdyn_bgq_gcc_64_none_none testA)


INSTALL (TARGETS
		symtab_group_testdyn_bgq_gcc_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_cc not present")
ENDIF()
