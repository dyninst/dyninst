# CMakeLists for symtab/bgxlc/64/dyn/none/none
set (CMAKE_C_FLAGS "${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}  -g -O0 ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "")
set (CMAKE_C_COMPILER "${M_mpi_xlc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
IF (NOT ${M_mpi_xlc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlc}"
	"${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}  -g -O0 "
	"${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink"
	"C"
	"mpi_xlc 64 dyn"
	MUTATEEmpixlc64dynC)

IF (MUTATEEmpixlc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
if(TARGET symtabAPI)
add_executable (symtab_group_test.dyn_bgxlc_64_none_none ${SOURCE_LIST_19})
foreach (f ${SOURCE_LIST_19})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(symtab_group_test.dyn_bgxlc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (symtab_group_test.dyn_bgxlc_64_none_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_bgxlc_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
