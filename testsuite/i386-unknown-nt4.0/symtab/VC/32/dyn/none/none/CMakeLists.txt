# CMakeLists for symtab/VC/32/dyn/none/none
set (CMAKE_C_FLAGS "${MUTATOR_DEFINES}   ${MUTATEE_CFLAGS_NATIVE}  /Od /Zi /MDd /D_DEBUG ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "")
set (CMAKE_C_COMPILER "${M_native_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_native_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_native_cc}"
	"${MUTATOR_DEFINES}   ${MUTATEE_CFLAGS_NATIVE}  /Od /Zi /MDd /D_DEBUG "
	"${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  "
	"C"
	"native_cc 32 dyn"
	MUTATEEnativecc32dynC)

IF (MUTATEEnativecc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
if(TARGET symtabAPI)
add_executable (symtab_group_test.dyn_VC_32_none_none ${SOURCE_LIST_29})
foreach (f ${SOURCE_LIST_29})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(symtab_group_test.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (symtab_group_test.dyn_VC_32_none_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_VC_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
