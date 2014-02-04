# CMakeLists for proccontrol/gcc/64/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"C"
	"gnu_cc 64 dyn"
	MUTATEEgnucc64dynC)

IF (MUTATEEgnucc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_addlibrary.dyn_gcc_64_none_none ${SOURCE_LIST_50})
set_source_files_properties(${SOURCE_LIST_50} PROPERTIES LANGUAGE C)
set_target_properties(pc_addlibrary.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_addlibrary.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_gcc_64_none_none ${SOURCE_LIST_39})
set_source_files_properties(${SOURCE_LIST_39} PROPERTIES LANGUAGE C)
set_target_properties(pc_breakpoint.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_breakpoint.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_gcc_64_none_none ${SOURCE_LIST_15})
set_source_files_properties(${SOURCE_LIST_15} PROPERTIES LANGUAGE C)
set_target_properties(pc_detach.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_detach.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_exec_targ.dyn_gcc_64_none_none ${SOURCE_LIST_28})
set_source_files_properties(${SOURCE_LIST_28} PROPERTIES LANGUAGE C)
set_target_properties(pc_exec_targ.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (pc_fork.dyn_gcc_64_none_none ${SOURCE_LIST_0})
set_source_files_properties(${SOURCE_LIST_0} PROPERTIES LANGUAGE C)
set_target_properties(pc_fork.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_fork.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_fork_exec.dyn_gcc_64_none_none ${SOURCE_LIST_63})
set_source_files_properties(${SOURCE_LIST_63} PROPERTIES LANGUAGE C)
set_target_properties(pc_fork_exec.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_fork_exec.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_gcc_64_none_none ${SOURCE_LIST_49})
set_source_files_properties(${SOURCE_LIST_49} PROPERTIES LANGUAGE C)
set_target_properties(pc_groups.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_groups.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_hw_breakpoint.dyn_gcc_64_none_none ${SOURCE_LIST_9})
set_source_files_properties(${SOURCE_LIST_9} PROPERTIES LANGUAGE C)
set_target_properties(pc_hw_breakpoint.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_hw_breakpoint.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_gcc_64_none_none ${SOURCE_LIST_79})
set_source_files_properties(${SOURCE_LIST_79} PROPERTIES LANGUAGE C)
set_target_properties(pc_irpc.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_irpc.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_gcc_64_none_none ${SOURCE_LIST_54})
set_source_files_properties(${SOURCE_LIST_54} PROPERTIES LANGUAGE C)
set_target_properties(pc_launch.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_launch.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_gcc_64_none_none ${SOURCE_LIST_34})
set_source_files_properties(${SOURCE_LIST_34} PROPERTIES LANGUAGE C)
set_target_properties(pc_library.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_library.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_gcc_64_none_none ${SOURCE_LIST_36})
set_source_files_properties(${SOURCE_LIST_36} PROPERTIES LANGUAGE C)
set_target_properties(pc_singlestep.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_singlestep.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_gcc_64_none_none ${SOURCE_LIST_13})
set_source_files_properties(${SOURCE_LIST_13} PROPERTIES LANGUAGE C)
set_target_properties(pc_stat.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_stat.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_gcc_64_none_none ${SOURCE_LIST_6})
set_source_files_properties(${SOURCE_LIST_6} PROPERTIES LANGUAGE C)
set_target_properties(pc_temp_detach.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_temp_detach.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_gcc_64_none_none ${SOURCE_LIST_32})
set_source_files_properties(${SOURCE_LIST_32} PROPERTIES LANGUAGE C)
set_target_properties(pc_terminate.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_terminate.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_gcc_64_none_none ${SOURCE_LIST_61})
set_source_files_properties(${SOURCE_LIST_61} PROPERTIES LANGUAGE C)
set_target_properties(pc_terminate_stopped.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_terminate_stopped.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_gcc_64_none_none ${SOURCE_LIST_46})
set_source_files_properties(${SOURCE_LIST_46} PROPERTIES LANGUAGE C)
set_target_properties(pc_thread.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_thread.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_gcc_64_none_none ${SOURCE_LIST_24})
set_source_files_properties(${SOURCE_LIST_24} PROPERTIES LANGUAGE C)
set_target_properties(pc_thread_cont.dyn_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_thread_cont.dyn_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_gcc_64_none_none
		pc_breakpoint.dyn_gcc_64_none_none
		pc_detach.dyn_gcc_64_none_none
		pc_exec_targ.dyn_gcc_64_none_none
		pc_fork.dyn_gcc_64_none_none
		pc_fork_exec.dyn_gcc_64_none_none
		pc_groups.dyn_gcc_64_none_none
		pc_hw_breakpoint.dyn_gcc_64_none_none
		pc_irpc.dyn_gcc_64_none_none
		pc_launch.dyn_gcc_64_none_none
		pc_library.dyn_gcc_64_none_none
		pc_singlestep.dyn_gcc_64_none_none
		pc_stat.dyn_gcc_64_none_none
		pc_temp_detach.dyn_gcc_64_none_none
		pc_terminate.dyn_gcc_64_none_none
		pc_terminate_stopped.dyn_gcc_64_none_none
		pc_thread.dyn_gcc_64_none_none
		pc_thread_cont.dyn_gcc_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cc not present")
ENDIF()
