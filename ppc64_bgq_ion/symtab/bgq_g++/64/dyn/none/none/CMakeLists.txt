# CMakeLists for symtab/bgq_g++/64/dyn/none/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_mpi_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
IF (NOT ${M_mpi_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic"
	"CXX"
	"mpi_cxx 64 dyn"
	MUTATEEmpicxx64dynCXX)

IF (MUTATEEmpicxx64dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (symtab_group_test.dyn_bgq_g++_64_none_none ${SOURCE_LIST_19})
set_source_files_properties(${SOURCE_LIST_19} PROPERTIES LANGUAGE CXX)
set_target_properties(symtab_group_test.dyn_bgq_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (symtab_group_test.dyn_bgq_g++_64_none_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_bgq_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_cxx not present")
ENDIF()
