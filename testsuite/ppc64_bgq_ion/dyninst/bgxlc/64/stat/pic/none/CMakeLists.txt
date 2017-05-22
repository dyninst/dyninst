# CMakeLists for dyninst/bgxlc/64/stat/pic/none
set (CMAKE_C_FLAGS "${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}  -g -O0 -qpic")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "")
set (CMAKE_C_COMPILER "${M_mpi_xlc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_mpi_xlc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlc}"
	"${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}  -g -O0 -qpic"
	"${MUTATEE_LDFLAGS_NATIVE}  "
	"C"
	"mpi_xlc 64 stat"
	MUTATEEmpixlc64statC)

IF (MUTATEEmpixlc64statC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
if(TARGET dyninstAPI)
add_executable (dyninst_group_test.stat_bgxlc_64_pic_none ${SOURCE_LIST_29})
foreach (f ${SOURCE_LIST_29})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(dyninst_group_test.stat_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (dyninst_group_test.stat_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_bgxlc_64_pic_none ${SOURCE_LIST_39})
foreach (f ${SOURCE_LIST_39})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_14.stat_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_29.stat_bgxlc_64_pic_none ${SOURCE_LIST_9})
foreach (f ${SOURCE_LIST_9})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE C)
	endif()
	endforeach()
set_target_properties(test1_29.stat_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)


INSTALL (TARGETS
		dyninst_group_test.stat_bgxlc_64_pic_none
		test1_14.stat_bgxlc_64_pic_none
		test1_29.stat_bgxlc_64_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
