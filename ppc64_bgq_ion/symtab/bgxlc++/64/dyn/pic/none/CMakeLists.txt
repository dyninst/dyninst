# CMakeLists for symtab/bgxlc++/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   -qpic")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_xlcxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
IF (NOT ${M_mpi_xlcxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlcxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   -qpic"
	"${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink"
	"cxx"
	"mpi_xlcxx 64 dyn"
	MUTATEEmpixlcxx64dyncxx)

IF (MUTATEEmpixlcxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (symtab_group_test.dyn_bgxlc++_64_pic_none ${SOURCE_LIST_19})
target_link_libraries (symtab_group_test.dyn_bgxlc++_64_pic_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_bgxlc++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_xlcxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_xlcxx not present")
ENDIF()
