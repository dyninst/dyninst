# CMakeLists for proccontrol/VC/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  -TC ${CFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_native_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${LDFLAGS} ${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
include (../../../../../../srclists.cmake)
add_executable (pc_addlibrary.dyn_cl_32_none_none.exe ${SOURCE_LIST_28})
target_link_libraries (pc_addlibrary.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_cl_32_none_none.exe ${SOURCE_LIST_23})
target_link_libraries (pc_breakpoint.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_cl_32_none_none.exe ${SOURCE_LIST_11})
target_link_libraries (pc_detach.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_cl_32_none_none.exe ${SOURCE_LIST_29})
target_link_libraries (pc_groups.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_cl_32_none_none.exe ${SOURCE_LIST_43})
target_link_libraries (pc_irpc.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_cl_32_none_none.exe ${SOURCE_LIST_44})
target_link_libraries (pc_launch.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_cl_32_none_none.exe ${SOURCE_LIST_20})
target_link_libraries (pc_library.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_mem_perm.dyn_cl_32_none_none.exe ${SOURCE_LIST_0})
target_link_libraries (pc_mem_perm.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_cl_32_none_none.exe ${SOURCE_LIST_22})
target_link_libraries (pc_singlestep.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_cl_32_none_none.exe ${SOURCE_LIST_9})
target_link_libraries (pc_stat.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_cl_32_none_none.exe ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detach.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_cl_32_none_none.exe ${SOURCE_LIST_18})
target_link_libraries (pc_terminate.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_cl_32_none_none.exe ${SOURCE_LIST_33})
target_link_libraries (pc_terminate_stopped.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_cl_32_none_none.exe ${SOURCE_LIST_25})
target_link_libraries (pc_thread.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_cl_32_none_none.exe ${SOURCE_LIST_13})
target_link_libraries (pc_thread_cont.dyn_cl_32_none_none.exe ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_cl_32_none_none.exe
		pc_breakpoint.dyn_cl_32_none_none.exe
		pc_detach.dyn_cl_32_none_none.exe
		pc_groups.dyn_cl_32_none_none.exe
		pc_irpc.dyn_cl_32_none_none.exe
		pc_launch.dyn_cl_32_none_none.exe
		pc_library.dyn_cl_32_none_none.exe
		pc_mem_perm.dyn_cl_32_none_none.exe
		pc_singlestep.dyn_cl_32_none_none.exe
		pc_stat.dyn_cl_32_none_none.exe
		pc_temp_detach.dyn_cl_32_none_none.exe
		pc_terminate.dyn_cl_32_none_none.exe
		pc_terminate_stopped.dyn_cl_32_none_none.exe
		pc_thread.dyn_cl_32_none_none.exe
		pc_thread_cont.dyn_cl_32_none_none.exe
	DESTINATION ${INSTALL_DIR})
