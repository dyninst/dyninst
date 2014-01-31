# CMakeLists for proccontrol/VC++/32/dyn/none/none
set (CMAKE_C_FLAGS "/D_DEBUG /I${PROJECT_SOURCE_DIR}/testsuite/src /I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  /TP ${CXXFLAGS_NORM}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_native_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_native_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_native_cc}"
	"/D_DEBUG /I${PROJECT_SOURCE_DIR}/testsuite/src /I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  /TP ${CXXFLAGS_NORM}   "
	"${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"cxx"
	"native_cc 32 dyn"
	MUTATEEnativecc32dyncxx)

IF (MUTATEEnativecc32dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
add_executable (pc_addlibrarydyn_VC++_32_none_none ${SOURCE_LIST_28})
target_link_libraries (pc_addlibrarydyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_breakpointdyn_VC++_32_none_none ${SOURCE_LIST_23})
target_link_libraries (pc_breakpointdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_detachdyn_VC++_32_none_none ${SOURCE_LIST_11})
target_link_libraries (pc_detachdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_groupsdyn_VC++_32_none_none ${SOURCE_LIST_27})
target_link_libraries (pc_groupsdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_irpcdyn_VC++_32_none_none ${SOURCE_LIST_43})
target_link_libraries (pc_irpcdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_launchdyn_VC++_32_none_none ${SOURCE_LIST_44})
target_link_libraries (pc_launchdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_librarydyn_VC++_32_none_none ${SOURCE_LIST_19})
target_link_libraries (pc_librarydyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_mem_permdyn_VC++_32_none_none ${SOURCE_LIST_0})
target_link_libraries (pc_mem_permdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_singlestepdyn_VC++_32_none_none ${SOURCE_LIST_21})
target_link_libraries (pc_singlestepdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_statdyn_VC++_32_none_none ${SOURCE_LIST_9})
target_link_libraries (pc_statdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_temp_detachdyn_VC++_32_none_none ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detachdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_terminatedyn_VC++_32_none_none ${SOURCE_LIST_17})
target_link_libraries (pc_terminatedyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_terminate_stoppeddyn_VC++_32_none_none ${SOURCE_LIST_33})
target_link_libraries (pc_terminate_stoppeddyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_threaddyn_VC++_32_none_none ${SOURCE_LIST_25})
target_link_libraries (pc_threaddyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_thread_contdyn_VC++_32_none_none ${SOURCE_LIST_13})
target_link_libraries (pc_thread_contdyn_VC++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)


INSTALL (TARGETS
		pc_addlibrarydyn_VC++_32_none_none
		pc_breakpointdyn_VC++_32_none_none
		pc_detachdyn_VC++_32_none_none
		pc_groupsdyn_VC++_32_none_none
		pc_irpcdyn_VC++_32_none_none
		pc_launchdyn_VC++_32_none_none
		pc_librarydyn_VC++_32_none_none
		pc_mem_permdyn_VC++_32_none_none
		pc_singlestepdyn_VC++_32_none_none
		pc_statdyn_VC++_32_none_none
		pc_temp_detachdyn_VC++_32_none_none
		pc_terminatedyn_VC++_32_none_none
		pc_terminate_stoppeddyn_VC++_32_none_none
		pc_threaddyn_VC++_32_none_none
		pc_thread_contdyn_VC++_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for native_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler native_cc not present")
ENDIF()
