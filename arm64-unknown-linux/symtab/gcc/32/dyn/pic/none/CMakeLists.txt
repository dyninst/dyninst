# CMakeLists for symtab/gcc/32/dyn/pic/none
set (CMAKE_C_FLAGS "${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}   -g -O0 -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}   -g -O0 -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"C"
	"gnu_cc 32 dyn"
	MUTATEEgnucc32dynC)

IF (MUTATEEgnucc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/arm64-unknown-linux/srclists.cmake)
if(TARGET symtabAPI)
add_executable (symtab_group_test.dyn_gcc_32_pic_none ${SOURCE_LIST_11})
foreach (f ${SOURCE_LIST_11})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(symtab_group_test.dyn_gcc_32_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (symtab_group_test.dyn_gcc_32_pic_none testA_m32)


INSTALL (TARGETS
		symtab_group_test.dyn_gcc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
