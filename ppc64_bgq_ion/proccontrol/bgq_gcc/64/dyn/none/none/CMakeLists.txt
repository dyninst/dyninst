# CMakeLists for proccontrol/bgq_gcc/64/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
IF (NOT ${M_mpi_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic"
	"C"
	"mpi_cc 64 dyn"
	MUTATEEmpicc64dynC)

IF (MUTATEEmpicc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (pc_addlibrary.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_37})
set_source_files_properties(${SOURCE_LIST_37} PROPERTIES LANGUAGE C)
set_target_properties(pc_addlibrary.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_addlibrary.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_30})
set_source_files_properties(${SOURCE_LIST_30} PROPERTIES LANGUAGE C)
set_target_properties(pc_breakpoint.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_breakpoint.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_11})
set_source_files_properties(${SOURCE_LIST_11} PROPERTIES LANGUAGE C)
set_target_properties(pc_detach.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_detach.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_36})
set_source_files_properties(${SOURCE_LIST_36} PROPERTIES LANGUAGE C)
set_target_properties(pc_groups.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_groups.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_60})
set_source_files_properties(${SOURCE_LIST_60} PROPERTIES LANGUAGE C)
set_target_properties(pc_irpc.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_irpc.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_61})
set_source_files_properties(${SOURCE_LIST_61} PROPERTIES LANGUAGE C)
set_target_properties(pc_launch.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_launch.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_25})
set_source_files_properties(${SOURCE_LIST_25} PROPERTIES LANGUAGE C)
set_target_properties(pc_library.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_library.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_27})
set_source_files_properties(${SOURCE_LIST_27} PROPERTIES LANGUAGE C)
set_target_properties(pc_singlestep.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_singlestep.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_4})
set_source_files_properties(${SOURCE_LIST_4} PROPERTIES LANGUAGE C)
set_target_properties(pc_stat.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_stat.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_23})
set_source_files_properties(${SOURCE_LIST_23} PROPERTIES LANGUAGE C)
set_target_properties(pc_terminate.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_terminate.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_45})
set_source_files_properties(${SOURCE_LIST_45} PROPERTIES LANGUAGE C)
set_target_properties(pc_terminate_stopped.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_terminate_stopped.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_bgq_gcc_64_none_none ${SOURCE_LIST_34})
set_source_files_properties(${SOURCE_LIST_34} PROPERTIES LANGUAGE C)
set_target_properties(pc_thread.dyn_bgq_gcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (pc_thread.dyn_bgq_gcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_bgq_gcc_64_none_none
		pc_breakpoint.dyn_bgq_gcc_64_none_none
		pc_detach.dyn_bgq_gcc_64_none_none
		pc_groups.dyn_bgq_gcc_64_none_none
		pc_irpc.dyn_bgq_gcc_64_none_none
		pc_launch.dyn_bgq_gcc_64_none_none
		pc_library.dyn_bgq_gcc_64_none_none
		pc_singlestep.dyn_bgq_gcc_64_none_none
		pc_stat.dyn_bgq_gcc_64_none_none
		pc_terminate.dyn_bgq_gcc_64_none_none
		pc_terminate_stopped.dyn_bgq_gcc_64_none_none
		pc_thread.dyn_bgq_gcc_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_cc not present")
ENDIF()
