# CMakeLists for dyninst/gcc/32/stat/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -static"
	"C"
	"gnu_cc 32 stat"
	MUTATEEgnucc32statC)

IF (MUTATEEgnucc32statC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc32_linux/srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_32_none_none ${SOURCE_LIST_58})
set_source_files_properties(${SOURCE_LIST_58} PROPERTIES LANGUAGE C)
set_target_properties(dyninst_group_test.stat_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (dyninst_group_test.stat_gcc_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_gcc_32_none_none ${SOURCE_LIST_41})
set_source_files_properties(${SOURCE_LIST_41} PROPERTIES LANGUAGE C)
set_target_properties(test1_14.stat_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_29.stat_gcc_32_none_none ${SOURCE_LIST_4})
set_source_files_properties(${SOURCE_LIST_4} PROPERTIES LANGUAGE C)
set_target_properties(test1_29.stat_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_32_none_none
		test1_14.stat_gcc_32_none_none
		test1_29.stat_gcc_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cc not present")
ENDIF()
