# CMakeLists for symtab/pgcc/32/dyn/none/none
set (CMAKE_C_FLAGS "${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test -g -O0 ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "")
set (CMAKE_C_COMPILER "${M_pg_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test ")
IF (NOT ${M_pg_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cc}"
	"${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test -g -O0 "
	"${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test "
	"C"
	"pg_cc 32 dyn"
	MUTATEEpgcc32dynC)

IF (MUTATEEpgcc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
if(TARGET symtabAPI)
add_executable (symtab_group_test.dyn_pgcc_32_none_none ${SOURCE_LIST_35})
foreach (f ${SOURCE_LIST_35})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(symtab_group_test.dyn_pgcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (symtab_group_test.dyn_pgcc_32_none_none testA_m32)


INSTALL (TARGETS
		symtab_group_test.dyn_pgcc_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
