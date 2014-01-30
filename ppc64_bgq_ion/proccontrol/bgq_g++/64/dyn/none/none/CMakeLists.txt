# CMakeLists for proccontrol/bgq_g++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
IF (NOT ${M_mpi_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic"
	"cxx"
	"mpi_cxx 64 dyn"
	MUTATEEmpicxx64dyncxx)

IF (MUTATEEmpicxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (pc_addlibrarydyn_bgq_g++_64_none_none ${SOURCE_LIST_37})
target_link_libraries (pc_addlibrarydyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpointdyn_bgq_g++_64_none_none ${SOURCE_LIST_30})
target_link_libraries (pc_breakpointdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detachdyn_bgq_g++_64_none_none ${SOURCE_LIST_11})
target_link_libraries (pc_detachdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groupsdyn_bgq_g++_64_none_none ${SOURCE_LIST_36})
target_link_libraries (pc_groupsdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpcdyn_bgq_g++_64_none_none ${SOURCE_LIST_60})
target_link_libraries (pc_irpcdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launchdyn_bgq_g++_64_none_none ${SOURCE_LIST_61})
target_link_libraries (pc_launchdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_librarydyn_bgq_g++_64_none_none ${SOURCE_LIST_25})
target_link_libraries (pc_librarydyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestepdyn_bgq_g++_64_none_none ${SOURCE_LIST_27})
target_link_libraries (pc_singlestepdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_statdyn_bgq_g++_64_none_none ${SOURCE_LIST_4})
target_link_libraries (pc_statdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminatedyn_bgq_g++_64_none_none ${SOURCE_LIST_23})
target_link_libraries (pc_terminatedyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stoppeddyn_bgq_g++_64_none_none ${SOURCE_LIST_45})
target_link_libraries (pc_terminate_stoppeddyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_threaddyn_bgq_g++_64_none_none ${SOURCE_LIST_34})
target_link_libraries (pc_threaddyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrarydyn_bgq_g++_64_none_none
		pc_breakpointdyn_bgq_g++_64_none_none
		pc_detachdyn_bgq_g++_64_none_none
		pc_groupsdyn_bgq_g++_64_none_none
		pc_irpcdyn_bgq_g++_64_none_none
		pc_launchdyn_bgq_g++_64_none_none
		pc_librarydyn_bgq_g++_64_none_none
		pc_singlestepdyn_bgq_g++_64_none_none
		pc_statdyn_bgq_g++_64_none_none
		pc_terminatedyn_bgq_g++_64_none_none
		pc_terminate_stoppeddyn_bgq_g++_64_none_none
		pc_threaddyn_bgq_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_cxx not present")
ENDIF()
