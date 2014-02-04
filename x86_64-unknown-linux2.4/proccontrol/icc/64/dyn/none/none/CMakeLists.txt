# CMakeLists for proccontrol/icc/64/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	"C"
	"intel_cc 64 dyn"
	MUTATEEintelcc64dynC)

IF (MUTATEEintelcc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_exec_targ.dyn_icc_64_none_none ${SOURCE_LIST_28})
foreach (f ${SOURCE_LIST_28})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(pc_exec_targ.dyn_icc_64_none_none PROPERTIES LINKER_LANGUAGE C)


INSTALL (TARGETS
		pc_exec_targ.dyn_icc_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
