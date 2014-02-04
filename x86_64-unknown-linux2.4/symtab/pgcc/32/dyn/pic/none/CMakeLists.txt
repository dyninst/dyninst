# CMakeLists for symtab/pgcc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test  -KPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test ")
IF (NOT ${M_pg_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}  -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test  -KPIC"
	"${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE} -tp px -m32 -Di386_unknown_linux2_4 -Dm32_test "
	"C"
	"pg_cc 32 dyn"
	MUTATEEpgcc32dynC)

IF (MUTATEEpgcc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (symtab_group_test.dyn_pgcc_32_pic_none ${SOURCE_LIST_35})
set_source_files_properties(${SOURCE_LIST_35} PROPERTIES LANGUAGE C)
set_target_properties(symtab_group_test.dyn_pgcc_32_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (symtab_group_test.dyn_pgcc_32_pic_none testA_m32)


INSTALL (TARGETS
		symtab_group_test.dyn_pgcc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for pg_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler pg_cc not present")
ENDIF()
