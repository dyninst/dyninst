# CMakeLists for proccontrol/VC/32/dyn/none/none
set (CMAKE_C_FLAGS "/D_DEBUG ${MUTATOR_DEFINES}   ${CFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_native_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  ")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_native_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_native_cc}"
	"/D_DEBUG ${MUTATOR_DEFINES}   ${CFLAGS}   "
	"${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  "
	"c"
	"native_cc 32 dyn"
	MUTATEEnativecc32dync)

IF (MUTATEEnativecc32dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
add_executable (pc_addlibrary.dyn_VC_32_none_none ${SOURCE_LIST_28})
target_link_libraries (pc_addlibrary.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_breakpoint.dyn_VC_32_none_none ${SOURCE_LIST_23})
target_link_libraries (pc_breakpoint.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_detach.dyn_VC_32_none_none ${SOURCE_LIST_11})
target_link_libraries (pc_detach.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_groups.dyn_VC_32_none_none ${SOURCE_LIST_27})
target_link_libraries (pc_groups.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_irpc.dyn_VC_32_none_none ${SOURCE_LIST_43})
target_link_libraries (pc_irpc.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_launch.dyn_VC_32_none_none ${SOURCE_LIST_44})
target_link_libraries (pc_launch.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_library.dyn_VC_32_none_none ${SOURCE_LIST_19})
target_link_libraries (pc_library.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_mem_perm.dyn_VC_32_none_none ${SOURCE_LIST_0})
target_link_libraries (pc_mem_perm.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_singlestep.dyn_VC_32_none_none ${SOURCE_LIST_21})
target_link_libraries (pc_singlestep.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_stat.dyn_VC_32_none_none ${SOURCE_LIST_9})
target_link_libraries (pc_stat.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_temp_detach.dyn_VC_32_none_none ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detach.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_terminate.dyn_VC_32_none_none ${SOURCE_LIST_17})
target_link_libraries (pc_terminate.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_terminate_stopped.dyn_VC_32_none_none ${SOURCE_LIST_33})
target_link_libraries (pc_terminate_stopped.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_thread.dyn_VC_32_none_none ${SOURCE_LIST_25})
target_link_libraries (pc_thread.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_thread_cont.dyn_VC_32_none_none ${SOURCE_LIST_13})
target_link_libraries (pc_thread_cont.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)


INSTALL (TARGETS
		pc_addlibrary.dyn_VC_32_none_none
		pc_breakpoint.dyn_VC_32_none_none
		pc_detach.dyn_VC_32_none_none
		pc_groups.dyn_VC_32_none_none
		pc_irpc.dyn_VC_32_none_none
		pc_launch.dyn_VC_32_none_none
		pc_library.dyn_VC_32_none_none
		pc_mem_perm.dyn_VC_32_none_none
		pc_singlestep.dyn_VC_32_none_none
		pc_stat.dyn_VC_32_none_none
		pc_temp_detach.dyn_VC_32_none_none
		pc_terminate.dyn_VC_32_none_none
		pc_terminate_stopped.dyn_VC_32_none_none
		pc_thread.dyn_VC_32_none_none
		pc_thread_cont.dyn_VC_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for native_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler native_cc not present")
ENDIF()
