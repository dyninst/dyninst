# CMakeLists for proccontrol/g++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"cxx"
	"gnu_cxx 64 dyn"
	MUTATEEgnucxx64dyncxx)

IF (MUTATEEgnucxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/amd64-unknown-freebsd7.2/srclists.cmake)
add_executable (pc_addlibrarydyn_g++_64_none_none ${SOURCE_LIST_48})
target_link_libraries (pc_addlibrarydyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpointdyn_g++_64_none_none ${SOURCE_LIST_37})
target_link_libraries (pc_breakpointdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detachdyn_g++_64_none_none ${SOURCE_LIST_15})
target_link_libraries (pc_detachdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groupsdyn_g++_64_none_none ${SOURCE_LIST_47})
target_link_libraries (pc_groupsdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpcdyn_g++_64_none_none ${SOURCE_LIST_73})
target_link_libraries (pc_irpcdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launchdyn_g++_64_none_none ${SOURCE_LIST_74})
target_link_libraries (pc_launchdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_librarydyn_g++_64_none_none ${SOURCE_LIST_32})
target_link_libraries (pc_librarydyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestepdyn_g++_64_none_none ${SOURCE_LIST_34})
target_link_libraries (pc_singlestepdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_statdyn_g++_64_none_none ${SOURCE_LIST_13})
target_link_libraries (pc_statdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detachdyn_g++_64_none_none ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detachdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminatedyn_g++_64_none_none ${SOURCE_LIST_30})
target_link_libraries (pc_terminatedyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stoppeddyn_g++_64_none_none ${SOURCE_LIST_56})
target_link_libraries (pc_terminate_stoppeddyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_threaddyn_g++_64_none_none ${SOURCE_LIST_44})
target_link_libraries (pc_threaddyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_contdyn_g++_64_none_none ${SOURCE_LIST_24})
target_link_libraries (pc_thread_contdyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrarydyn_g++_64_none_none
		pc_breakpointdyn_g++_64_none_none
		pc_detachdyn_g++_64_none_none
		pc_groupsdyn_g++_64_none_none
		pc_irpcdyn_g++_64_none_none
		pc_launchdyn_g++_64_none_none
		pc_librarydyn_g++_64_none_none
		pc_singlestepdyn_g++_64_none_none
		pc_statdyn_g++_64_none_none
		pc_temp_detachdyn_g++_64_none_none
		pc_terminatedyn_g++_64_none_none
		pc_terminate_stoppeddyn_g++_64_none_none
		pc_threaddyn_g++_64_none_none
		pc_thread_contdyn_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
