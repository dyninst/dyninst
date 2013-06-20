# CMakeLists for proccontrol/bgq_g++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_xx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (pc_addlibrary.dyn_bgq_g++_64_none_none ${SOURCE_LIST_37})
target_link_libraries (pc_addlibrary.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_bgq_g++_64_none_none ${SOURCE_LIST_30})
target_link_libraries (pc_breakpoint.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_bgq_g++_64_none_none ${SOURCE_LIST_11})
target_link_libraries (pc_detach.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_bgq_g++_64_none_none ${SOURCE_LIST_36})
target_link_libraries (pc_groups.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_bgq_g++_64_none_none ${SOURCE_LIST_59})
target_link_libraries (pc_irpc.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_bgq_g++_64_none_none ${SOURCE_LIST_60})
target_link_libraries (pc_launch.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_bgq_g++_64_none_none ${SOURCE_LIST_25})
target_link_libraries (pc_library.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_bgq_g++_64_none_none ${SOURCE_LIST_27})
target_link_libraries (pc_singlestep.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_bgq_g++_64_none_none ${SOURCE_LIST_4})
target_link_libraries (pc_stat.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_bgq_g++_64_none_none ${SOURCE_LIST_23})
target_link_libraries (pc_terminate.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_bgq_g++_64_none_none ${SOURCE_LIST_44})
target_link_libraries (pc_terminate_stopped.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_bgq_g++_64_none_none ${SOURCE_LIST_34})
target_link_libraries (pc_thread.dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_bgq_g++_64_none_none
		pc_breakpoint.dyn_bgq_g++_64_none_none
		pc_detach.dyn_bgq_g++_64_none_none
		pc_groups.dyn_bgq_g++_64_none_none
		pc_irpc.dyn_bgq_g++_64_none_none
		pc_launch.dyn_bgq_g++_64_none_none
		pc_library.dyn_bgq_g++_64_none_none
		pc_singlestep.dyn_bgq_g++_64_none_none
		pc_stat.dyn_bgq_g++_64_none_none
		pc_terminate.dyn_bgq_g++_64_none_none
		pc_terminate_stopped.dyn_bgq_g++_64_none_none
		pc_thread.dyn_bgq_g++_64_none_none
	DESTINATION ${INSTALL_DIR})
