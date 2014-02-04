# CMakeLists for proccontrol/g++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"cxx"
	"gnu_cxx 64 dyn"
	MUTATEEgnucxx64dyncxx)

IF (MUTATEEgnucxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (pc_addlibrary.dyn_g++_64_none_none ${SOURCE_LIST_50})
target_link_libraries (pc_addlibrary.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_g++_64_none_none ${SOURCE_LIST_39})
target_link_libraries (pc_breakpoint.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_g++_64_none_none ${SOURCE_LIST_15})
target_link_libraries (pc_detach.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_exec_targ.dyn_g++_64_none_none ${SOURCE_LIST_28})
add_executable (pc_fork.dyn_g++_64_none_none ${SOURCE_LIST_0})
target_link_libraries (pc_fork.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_fork_exec.dyn_g++_64_none_none ${SOURCE_LIST_63})
target_link_libraries (pc_fork_exec.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_g++_64_none_none ${SOURCE_LIST_49})
target_link_libraries (pc_groups.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_hw_breakpoint.dyn_g++_64_none_none ${SOURCE_LIST_9})
target_link_libraries (pc_hw_breakpoint.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_g++_64_none_none ${SOURCE_LIST_79})
target_link_libraries (pc_irpc.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_g++_64_none_none ${SOURCE_LIST_54})
target_link_libraries (pc_launch.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_g++_64_none_none ${SOURCE_LIST_34})
target_link_libraries (pc_library.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_g++_64_none_none ${SOURCE_LIST_36})
target_link_libraries (pc_singlestep.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_g++_64_none_none ${SOURCE_LIST_13})
target_link_libraries (pc_stat.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_g++_64_none_none ${SOURCE_LIST_6})
target_link_libraries (pc_temp_detach.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_g++_64_none_none ${SOURCE_LIST_32})
target_link_libraries (pc_terminate.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_g++_64_none_none ${SOURCE_LIST_61})
target_link_libraries (pc_terminate_stopped.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_g++_64_none_none ${SOURCE_LIST_46})
target_link_libraries (pc_thread.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_g++_64_none_none ${SOURCE_LIST_24})
target_link_libraries (pc_thread_cont.dyn_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_g++_64_none_none
		pc_breakpoint.dyn_g++_64_none_none
		pc_detach.dyn_g++_64_none_none
		pc_exec_targ.dyn_g++_64_none_none
		pc_fork.dyn_g++_64_none_none
		pc_fork_exec.dyn_g++_64_none_none
		pc_groups.dyn_g++_64_none_none
		pc_hw_breakpoint.dyn_g++_64_none_none
		pc_irpc.dyn_g++_64_none_none
		pc_launch.dyn_g++_64_none_none
		pc_library.dyn_g++_64_none_none
		pc_singlestep.dyn_g++_64_none_none
		pc_stat.dyn_g++_64_none_none
		pc_temp_detach.dyn_g++_64_none_none
		pc_terminate.dyn_g++_64_none_none
		pc_terminate_stopped.dyn_g++_64_none_none
		pc_thread.dyn_g++_64_none_none
		pc_thread_cont.dyn_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
