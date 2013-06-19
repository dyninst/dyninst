# CMakeLists for proccontrol/gcc/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (pc_addlibrary.dyn_gcc_32_none_none ${SOURCE_LIST_51})
target_link_libraries (pc_addlibrary.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_gcc_32_none_none ${SOURCE_LIST_70})
target_link_libraries (pc_breakpoint.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_gcc_32_none_none ${SOURCE_LIST_61})
target_link_libraries (pc_detach.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_exec_targ.dyn_gcc_32_none_none ${SOURCE_LIST_11})
add_executable (pc_fork.dyn_gcc_32_none_none ${SOURCE_LIST_81})
target_link_libraries (pc_fork.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_fork_exec.dyn_gcc_32_none_none ${SOURCE_LIST_98})
target_link_libraries (pc_fork_exec.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_gcc_32_none_none ${SOURCE_LIST_93})
target_link_libraries (pc_groups.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_hw_breakpoint.dyn_gcc_32_none_none ${SOURCE_LIST_4})
target_link_libraries (pc_hw_breakpoint.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_gcc_32_none_none ${SOURCE_LIST_80})
target_link_libraries (pc_irpc.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_gcc_32_none_none ${SOURCE_LIST_21})
target_link_libraries (pc_launch.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_gcc_32_none_none ${SOURCE_LIST_14})
target_link_libraries (pc_library.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_gcc_32_none_none ${SOURCE_LIST_43})
target_link_libraries (pc_singlestep.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_gcc_32_none_none ${SOURCE_LIST_85})
target_link_libraries (pc_stat.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_gcc_32_none_none ${SOURCE_LIST_3})
target_link_libraries (pc_temp_detach.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_gcc_32_none_none ${SOURCE_LIST_41})
target_link_libraries (pc_terminate.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_gcc_32_none_none ${SOURCE_LIST_96})
target_link_libraries (pc_terminate_stopped.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_gcc_32_none_none ${SOURCE_LIST_17})
target_link_libraries (pc_thread.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_gcc_32_none_none ${SOURCE_LIST_8})
target_link_libraries (pc_thread_cont.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_gcc_32_none_none
		pc_breakpoint.dyn_gcc_32_none_none
		pc_detach.dyn_gcc_32_none_none
		pc_exec_targ.dyn_gcc_32_none_none
		pc_fork.dyn_gcc_32_none_none
		pc_fork_exec.dyn_gcc_32_none_none
		pc_groups.dyn_gcc_32_none_none
		pc_hw_breakpoint.dyn_gcc_32_none_none
		pc_irpc.dyn_gcc_32_none_none
		pc_launch.dyn_gcc_32_none_none
		pc_library.dyn_gcc_32_none_none
		pc_singlestep.dyn_gcc_32_none_none
		pc_stat.dyn_gcc_32_none_none
		pc_temp_detach.dyn_gcc_32_none_none
		pc_terminate.dyn_gcc_32_none_none
		pc_terminate_stopped.dyn_gcc_32_none_none
		pc_thread.dyn_gcc_32_none_none
		pc_thread_cont.dyn_gcc_32_none_none
	DESTINATION ${INSTALL_DIR})
