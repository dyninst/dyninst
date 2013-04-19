# CMakeLists for proccontrol/bgq_gcc/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (pc_addlibrary.dyn_mpicc_64_none_none ${SOURCE_LIST_36})
target_link_libraries (pc_addlibrary.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_breakpoint.dyn_mpicc_64_none_none ${SOURCE_LIST_30})
target_link_libraries (pc_breakpoint.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_detach.dyn_mpicc_64_none_none ${SOURCE_LIST_11})
target_link_libraries (pc_detach.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_groups.dyn_mpicc_64_none_none ${SOURCE_LIST_35})
target_link_libraries (pc_groups.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_irpc.dyn_mpicc_64_none_none ${SOURCE_LIST_59})
target_link_libraries (pc_irpc.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_launch.dyn_mpicc_64_none_none ${SOURCE_LIST_60})
target_link_libraries (pc_launch.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_library.dyn_mpicc_64_none_none ${SOURCE_LIST_25})
target_link_libraries (pc_library.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_singlestep.dyn_mpicc_64_none_none ${SOURCE_LIST_27})
target_link_libraries (pc_singlestep.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_stat.dyn_mpicc_64_none_none ${SOURCE_LIST_4})
target_link_libraries (pc_stat.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_terminate.dyn_mpicc_64_none_none ${SOURCE_LIST_23})
target_link_libraries (pc_terminate.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_terminate_stopped.dyn_mpicc_64_none_none ${SOURCE_LIST_43})
target_link_libraries (pc_terminate_stopped.dyn_mpicc_64_none_none dl pthread)
add_executable (pc_thread.dyn_mpicc_64_none_none ${SOURCE_LIST_34})
target_link_libraries (pc_thread.dyn_mpicc_64_none_none dl pthread)


INSTALL (TARGETS
		pc_addlibrary.dyn_mpicc_64_none_none
		pc_breakpoint.dyn_mpicc_64_none_none
		pc_detach.dyn_mpicc_64_none_none
		pc_groups.dyn_mpicc_64_none_none
		pc_irpc.dyn_mpicc_64_none_none
		pc_launch.dyn_mpicc_64_none_none
		pc_library.dyn_mpicc_64_none_none
		pc_singlestep.dyn_mpicc_64_none_none
		pc_stat.dyn_mpicc_64_none_none
		pc_terminate.dyn_mpicc_64_none_none
		pc_terminate_stopped.dyn_mpicc_64_none_none
		pc_thread.dyn_mpicc_64_none_none
	DESTINATION ${INSTALL_DIR})
