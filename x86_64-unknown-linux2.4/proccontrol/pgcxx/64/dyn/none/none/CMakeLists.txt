# CMakeLists for proccontrol/pgcxx/64/dyn/none/none
set (CMAKE_CXX_FLAGS "${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} ${MUTATEE_CXXFLAGS_NATIVE}   -g -O0 ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set (CMAKE_CXX_COMPILER "${M_pg_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_pg_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cxx}"
	"${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} ${MUTATEE_CXXFLAGS_NATIVE}   -g -O0 "
	"${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"CXX"
	"pg_cxx 64 dyn"
	MUTATEEpgcxx64dynCXX)

IF (MUTATEEpgcxx64dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
if(TARGET pcontrol)
add_executable (pc_exec_targ.dyn_pgcxx_64_none_none ${SOURCE_LIST_28})
foreach (f ${SOURCE_LIST_28})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(pc_exec_targ.dyn_pgcxx_64_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		pc_exec_targ.dyn_pgcxx_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
