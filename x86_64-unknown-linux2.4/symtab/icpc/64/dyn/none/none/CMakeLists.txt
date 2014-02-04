# CMakeLists for symtab/icpc/64/dyn/none/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS}  ${MUTATEE_CXXFLAGS_GNU}     ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_intel_CC}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_CC} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_CC}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS}  ${MUTATEE_CXXFLAGS_GNU}     "
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"CXX"
	"intel_CC 64 dyn"
	MUTATEEintelCC64dynCXX)

IF (MUTATEEintelCC64dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (symtab_group_test.dyn_icpc_64_none_none ${SOURCE_LIST_35})
set_source_files_properties(${SOURCE_LIST_35} PROPERTIES LANGUAGE CXX)
set_target_properties(symtab_group_test.dyn_icpc_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (symtab_group_test.dyn_icpc_64_none_none testA)
add_executable (test_exception.dyn_icpc_64_none_none ${SOURCE_LIST_70})
set_source_files_properties(${SOURCE_LIST_70} PROPERTIES LANGUAGE CXX)
set_target_properties(test_exception.dyn_icpc_64_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		symtab_group_test.dyn_icpc_64_none_none
		test_exception.dyn_icpc_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_CC failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_CC not present")
ENDIF()
