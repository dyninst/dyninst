# CMakeLists for proccontrol/bgxlc++/64/stat/pic/none
set (CMAKE_CXX_FLAGS "${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}  -g -O0 -qpic")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set (CMAKE_CXX_COMPILER "${M_mpi_xlcxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_mpi_xlcxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlcxx}"
	"${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}  -g -O0 -qpic"
	"${MUTATEE_LDFLAGS_NATIVE}  "
	"CXX"
	"mpi_xlcxx 64 stat"
	MUTATEEmpixlcxx64statCXX)

IF (MUTATEEmpixlcxx64statCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (pc_tls.stat_bgxlc++_64_pic_none ${SOURCE_LIST_51})
foreach (f ${SOURCE_LIST_51})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(pc_tls.stat_bgxlc++_64_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_tls.stat_bgxlc++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_tls.stat_bgxlc++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
