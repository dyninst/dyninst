# CMakeLists for proccontrol/icpc/32/dyn/none/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS}  ${MUTATEE_CXXFLAGS_GNU}     ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_intel_CC}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_CC} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_CC}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS}  ${MUTATEE_CXXFLAGS_GNU}     "
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"CXX"
	"intel_CC 32 dyn"
	MUTATEEintelCC32dynCXX)

IF (MUTATEEintelCC32dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targ.dyn_icpc_32_none_none ${SOURCE_LIST_11})
set_source_files_properties(${SOURCE_LIST_11} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_exec_targ.dyn_icpc_32_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		pc_exec_targ.dyn_icpc_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_CC failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_CC not present")
ENDIF()
