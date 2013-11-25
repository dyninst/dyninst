# CMakeLists for symtab/iCC/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS} -x c++ -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}     ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_CC}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_CC} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_CC}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  ${CXXFLAGS} -x c++ -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}     "
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	MUTATEEintelCC32dyn)

IF (MUTATEEintelCC32dyn)
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (symtab_group_test.dyn_iCC_32_none_none ${SOURCE_LIST_27})
target_link_libraries (symtab_group_test.dyn_iCC_32_none_none testA_m32)
add_executable (test_exception.dyn_iCC_32_none_none ${SOURCE_LIST_76})


INSTALL (TARGETS
		symtab_group_test.dyn_iCC_32_none_none
		test_exception.dyn_iCC_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
