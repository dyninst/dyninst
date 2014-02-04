# CMakeLists for proccontrol/g++/32/dyn/pic/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_gnu_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"CXX"
	"gnu_cxx 32 dyn"
	MUTATEEgnucxx32dynCXX)

IF (MUTATEEgnucxx32dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc32_linux/srclists.cmake)
add_executable (pc_addlibrary.dyn_g++_32_pic_none ${SOURCE_LIST_39})
set_source_files_properties(${SOURCE_LIST_39} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_addlibrary.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_addlibrary.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_g++_32_pic_none ${SOURCE_LIST_32})
set_source_files_properties(${SOURCE_LIST_32} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_breakpoint.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_breakpoint.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_g++_32_pic_none ${SOURCE_LIST_10})
set_source_files_properties(${SOURCE_LIST_10} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_detach.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_detach.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_exec_targ.dyn_g++_32_pic_none ${SOURCE_LIST_22})
set_source_files_properties(${SOURCE_LIST_22} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_exec_targ.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (pc_fork.dyn_g++_32_pic_none ${SOURCE_LIST_0})
set_source_files_properties(${SOURCE_LIST_0} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_fork.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_fork.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_fork_exec.dyn_g++_32_pic_none ${SOURCE_LIST_51})
set_source_files_properties(${SOURCE_LIST_51} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_fork_exec.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_fork_exec.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_g++_32_pic_none ${SOURCE_LIST_38})
set_source_files_properties(${SOURCE_LIST_38} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_groups.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_groups.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_g++_32_pic_none ${SOURCE_LIST_67})
set_source_files_properties(${SOURCE_LIST_67} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_irpc.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_irpc.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_g++_32_pic_none ${SOURCE_LIST_42})
set_source_files_properties(${SOURCE_LIST_42} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_launch.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_launch.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_g++_32_pic_none ${SOURCE_LIST_27})
set_source_files_properties(${SOURCE_LIST_27} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_library.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_library.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_g++_32_pic_none ${SOURCE_LIST_29})
set_source_files_properties(${SOURCE_LIST_29} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_singlestep.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_singlestep.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_g++_32_pic_none ${SOURCE_LIST_8})
set_source_files_properties(${SOURCE_LIST_8} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_stat.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_stat.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_g++_32_pic_none ${SOURCE_LIST_5})
set_source_files_properties(${SOURCE_LIST_5} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_temp_detach.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_temp_detach.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_g++_32_pic_none ${SOURCE_LIST_25})
set_source_files_properties(${SOURCE_LIST_25} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_terminate.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_terminate.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_g++_32_pic_none ${SOURCE_LIST_49})
set_source_files_properties(${SOURCE_LIST_49} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_terminate_stopped.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_terminate_stopped.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_g++_32_pic_none ${SOURCE_LIST_37})
set_source_files_properties(${SOURCE_LIST_37} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_thread.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_thread.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_g++_32_pic_none ${SOURCE_LIST_18})
set_source_files_properties(${SOURCE_LIST_18} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_thread_cont.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_thread_cont.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_g++_32_pic_none
		pc_breakpoint.dyn_g++_32_pic_none
		pc_detach.dyn_g++_32_pic_none
		pc_exec_targ.dyn_g++_32_pic_none
		pc_fork.dyn_g++_32_pic_none
		pc_fork_exec.dyn_g++_32_pic_none
		pc_groups.dyn_g++_32_pic_none
		pc_irpc.dyn_g++_32_pic_none
		pc_launch.dyn_g++_32_pic_none
		pc_library.dyn_g++_32_pic_none
		pc_singlestep.dyn_g++_32_pic_none
		pc_stat.dyn_g++_32_pic_none
		pc_temp_detach.dyn_g++_32_pic_none
		pc_terminate.dyn_g++_32_pic_none
		pc_terminate_stopped.dyn_g++_32_pic_none
		pc_thread.dyn_g++_32_pic_none
		pc_thread_cont.dyn_g++_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
