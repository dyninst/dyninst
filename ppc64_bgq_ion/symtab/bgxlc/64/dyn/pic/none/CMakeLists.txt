# CMakeLists for symtab/bgxlc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}   -qpic")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_xlc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
IF (NOT ${M_mpi_xlc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}   -qpic"
	"${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink"
	"c"
	"mpi_xlc 64 dyn"
	MUTATEEmpixlc64dync)

IF (MUTATEEmpixlc64dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (symtab_group_testdyn_bgxlc_64_pic_none ${SOURCE_LIST_19})
target_link_libraries (symtab_group_testdyn_bgxlc_64_pic_none testA)


INSTALL (TARGETS
		symtab_group_testdyn_bgxlc_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_xlc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_xlc not present")
ENDIF()
