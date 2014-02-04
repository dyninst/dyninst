# CMakeLists for symtab/gcc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"C"
	"gnu_cc 32 dyn"
	MUTATEEgnucc32dynC)

IF (MUTATEEgnucc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc32_linux/srclists.cmake)
add_executable (symtab_group_test.dyn_gcc_32_pic_none ${SOURCE_LIST_52})
set_source_files_properties(${SOURCE_LIST_52} PROPERTIES LANGUAGE C)
set_target_properties(symtab_group_test.dyn_gcc_32_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (symtab_group_test.dyn_gcc_32_pic_none testA_m32)


INSTALL (TARGETS
		symtab_group_test.dyn_gcc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cc not present")
ENDIF()
