# CMakeLists for symtab/pgcxx/32/dyn/pic/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} ${MUTATEE_CXXFLAGS_NATIVE}    -KPIC")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_pg_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_pg_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} ${MUTATEE_CXXFLAGS_NATIVE}    -KPIC"
	"${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"CXX"
	"pg_cxx 32 dyn"
	MUTATEEpgcxx32dynCXX)

IF (MUTATEEpgcxx32dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (symtab_group_test.dyn_pgcxx_32_pic_none ${SOURCE_LIST_27})
set_source_files_properties(${SOURCE_LIST_27} PROPERTIES LANGUAGE CXX)
set_target_properties(symtab_group_test.dyn_pgcxx_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (symtab_group_test.dyn_pgcxx_32_pic_none testA_m32)


INSTALL (TARGETS
		symtab_group_test.dyn_pgcxx_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for pg_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler pg_cxx not present")
ENDIF()
