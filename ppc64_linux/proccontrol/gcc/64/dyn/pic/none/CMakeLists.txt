# CMakeLists for proccontrol/gcc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -m64  -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m64 -Wl,-export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (pc_addlibrary.dyn_gcc_64_pic_none ${SOURCE_LIST_49})
target_link_libraries (pc_addlibrary.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_breakpoint.dyn_gcc_64_pic_none ${SOURCE_LIST_32})
target_link_libraries (pc_breakpoint.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_detach.dyn_gcc_64_pic_none ${SOURCE_LIST_11})
target_link_libraries (pc_detach.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_exec_targ.dyn_gcc_64_pic_none ${SOURCE_LIST_22})
add_executable (pc_fork.dyn_gcc_64_pic_none ${SOURCE_LIST_0})
target_link_libraries (pc_fork.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_fork_exec.dyn_gcc_64_pic_none ${SOURCE_LIST_51})
target_link_libraries (pc_fork_exec.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_groups.dyn_gcc_64_pic_none ${SOURCE_LIST_38})
target_link_libraries (pc_groups.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_irpc.dyn_gcc_64_pic_none ${SOURCE_LIST_66})
target_link_libraries (pc_irpc.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_launch.dyn_gcc_64_pic_none ${SOURCE_LIST_42})
target_link_libraries (pc_launch.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_library.dyn_gcc_64_pic_none ${SOURCE_LIST_27})
target_link_libraries (pc_library.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_singlestep.dyn_gcc_64_pic_none ${SOURCE_LIST_29})
target_link_libraries (pc_singlestep.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_stat.dyn_gcc_64_pic_none ${SOURCE_LIST_8})
target_link_libraries (pc_stat.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_temp_detach.dyn_gcc_64_pic_none ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detach.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_terminate.dyn_gcc_64_pic_none ${SOURCE_LIST_25})
target_link_libraries (pc_terminate.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_terminate_stopped.dyn_gcc_64_pic_none ${SOURCE_LIST_48})
target_link_libraries (pc_terminate_stopped.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_thread.dyn_gcc_64_pic_none ${SOURCE_LIST_37})
target_link_libraries (pc_thread.dyn_gcc_64_pic_none dl pthread)
add_executable (pc_thread_cont.dyn_gcc_64_pic_none ${SOURCE_LIST_19})
target_link_libraries (pc_thread_cont.dyn_gcc_64_pic_none dl pthread)


INSTALL (TARGETS
		pc_addlibrary.dyn_gcc_64_pic_none
		pc_breakpoint.dyn_gcc_64_pic_none
		pc_detach.dyn_gcc_64_pic_none
		pc_exec_targ.dyn_gcc_64_pic_none
		pc_fork.dyn_gcc_64_pic_none
		pc_fork_exec.dyn_gcc_64_pic_none
		pc_groups.dyn_gcc_64_pic_none
		pc_irpc.dyn_gcc_64_pic_none
		pc_launch.dyn_gcc_64_pic_none
		pc_library.dyn_gcc_64_pic_none
		pc_singlestep.dyn_gcc_64_pic_none
		pc_stat.dyn_gcc_64_pic_none
		pc_temp_detach.dyn_gcc_64_pic_none
		pc_terminate.dyn_gcc_64_pic_none
		pc_terminate_stopped.dyn_gcc_64_pic_none
		pc_thread.dyn_gcc_64_pic_none
		pc_thread_cont.dyn_gcc_64_pic_none
	DESTINATION ${INSTALL_DIR})
