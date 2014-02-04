# CMakeLists for proccontrol/VC/32/dyn/none/none
set (CMAKE_C_FLAGS "/D_DEBUG ${MUTATOR_DEFINES}   ${CFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_native_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_native_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_native_cc}"
	"/D_DEBUG ${MUTATOR_DEFINES}   ${CFLAGS}   "
	"${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  "
	"C"
	"native_cc 32 dyn"
	MUTATEEnativecc32dynC)

IF (MUTATEEnativecc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
add_executable (pc_addlibrary.dyn_VC_32_none_none ${SOURCE_LIST_28})
set_source_files_properties(${SOURCE_LIST_28} PROPERTIES LANGUAGE C)
set_target_properties(pc_addlibrary.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_addlibrary.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_breakpoint.dyn_VC_32_none_none ${SOURCE_LIST_23})
set_source_files_properties(${SOURCE_LIST_23} PROPERTIES LANGUAGE C)
set_target_properties(pc_breakpoint.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_breakpoint.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_detach.dyn_VC_32_none_none ${SOURCE_LIST_11})
set_source_files_properties(${SOURCE_LIST_11} PROPERTIES LANGUAGE C)
set_target_properties(pc_detach.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_detach.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_groups.dyn_VC_32_none_none ${SOURCE_LIST_27})
set_source_files_properties(${SOURCE_LIST_27} PROPERTIES LANGUAGE C)
set_target_properties(pc_groups.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_groups.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_irpc.dyn_VC_32_none_none ${SOURCE_LIST_43})
set_source_files_properties(${SOURCE_LIST_43} PROPERTIES LANGUAGE C)
set_target_properties(pc_irpc.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_irpc.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_launch.dyn_VC_32_none_none ${SOURCE_LIST_44})
set_source_files_properties(${SOURCE_LIST_44} PROPERTIES LANGUAGE C)
set_target_properties(pc_launch.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_launch.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_library.dyn_VC_32_none_none ${SOURCE_LIST_19})
set_source_files_properties(${SOURCE_LIST_19} PROPERTIES LANGUAGE C)
set_target_properties(pc_library.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_library.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_mem_perm.dyn_VC_32_none_none ${SOURCE_LIST_0})
set_source_files_properties(${SOURCE_LIST_0} PROPERTIES LANGUAGE C)
set_target_properties(pc_mem_perm.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_mem_perm.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_singlestep.dyn_VC_32_none_none ${SOURCE_LIST_21})
set_source_files_properties(${SOURCE_LIST_21} PROPERTIES LANGUAGE C)
set_target_properties(pc_singlestep.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_singlestep.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_stat.dyn_VC_32_none_none ${SOURCE_LIST_9})
set_source_files_properties(${SOURCE_LIST_9} PROPERTIES LANGUAGE C)
set_target_properties(pc_stat.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_stat.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_temp_detach.dyn_VC_32_none_none ${SOURCE_LIST_5})
set_source_files_properties(${SOURCE_LIST_5} PROPERTIES LANGUAGE C)
set_target_properties(pc_temp_detach.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_temp_detach.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_terminate.dyn_VC_32_none_none ${SOURCE_LIST_17})
set_source_files_properties(${SOURCE_LIST_17} PROPERTIES LANGUAGE C)
set_target_properties(pc_terminate.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_terminate.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_terminate_stopped.dyn_VC_32_none_none ${SOURCE_LIST_33})
set_source_files_properties(${SOURCE_LIST_33} PROPERTIES LANGUAGE C)
set_target_properties(pc_terminate_stopped.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_terminate_stopped.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_thread.dyn_VC_32_none_none ${SOURCE_LIST_25})
set_source_files_properties(${SOURCE_LIST_25} PROPERTIES LANGUAGE C)
set_target_properties(pc_thread.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_thread.dyn_VC_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ws2_32)
add_executable (pc_thread_cont.dyn_VC_32_none_none ${SOURCE_LIST_13})
set_source_files_properties(${SOURCE_LIST_13} PROPERTIES LANGUAGE C)
set_target_properties(pc_thread_cont.dyn_VC_32_none_none PROPERTIES LINKER_LANGUAGE C)
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
