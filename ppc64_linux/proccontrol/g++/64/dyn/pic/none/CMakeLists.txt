# CMakeLists for proccontrol/g++/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}  -m64  -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m64 -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}  -m64  -fPIC"
	"${MUTATEE_LDFLAGS_GNU} -m64 -Wl,-export-dynamic"
	"cxx"
	MUTATEEgnucxx64dyn)

IF (MUTATEEgnucxx64dyn MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_linux/srclists.cmake)
add_executable (pc_addlibrary.dyn_g++_64_pic_none ${SOURCE_LIST_41})
target_link_libraries (pc_addlibrary.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_g++_64_pic_none ${SOURCE_LIST_34})
target_link_libraries (pc_breakpoint.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_g++_64_pic_none ${SOURCE_LIST_12})
target_link_libraries (pc_detach.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_exec_targ.dyn_g++_64_pic_none ${SOURCE_LIST_24})
add_executable (pc_fork.dyn_g++_64_pic_none ${SOURCE_LIST_0})
target_link_libraries (pc_fork.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_fork_exec.dyn_g++_64_pic_none ${SOURCE_LIST_53})
target_link_libraries (pc_fork_exec.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_g++_64_pic_none ${SOURCE_LIST_40})
target_link_libraries (pc_groups.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_g++_64_pic_none ${SOURCE_LIST_67})
target_link_libraries (pc_irpc.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_g++_64_pic_none ${SOURCE_LIST_44})
target_link_libraries (pc_launch.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_g++_64_pic_none ${SOURCE_LIST_29})
target_link_libraries (pc_library.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_g++_64_pic_none ${SOURCE_LIST_31})
target_link_libraries (pc_singlestep.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_g++_64_pic_none ${SOURCE_LIST_10})
target_link_libraries (pc_stat.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_g++_64_pic_none ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detach.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_g++_64_pic_none ${SOURCE_LIST_27})
target_link_libraries (pc_terminate.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_g++_64_pic_none ${SOURCE_LIST_51})
target_link_libraries (pc_terminate_stopped.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_g++_64_pic_none ${SOURCE_LIST_39})
target_link_libraries (pc_thread.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_g++_64_pic_none ${SOURCE_LIST_20})
target_link_libraries (pc_thread_cont.dyn_g++_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_g++_64_pic_none
		pc_breakpoint.dyn_g++_64_pic_none
		pc_detach.dyn_g++_64_pic_none
		pc_exec_targ.dyn_g++_64_pic_none
		pc_fork.dyn_g++_64_pic_none
		pc_fork_exec.dyn_g++_64_pic_none
		pc_groups.dyn_g++_64_pic_none
		pc_irpc.dyn_g++_64_pic_none
		pc_launch.dyn_g++_64_pic_none
		pc_library.dyn_g++_64_pic_none
		pc_singlestep.dyn_g++_64_pic_none
		pc_stat.dyn_g++_64_pic_none
		pc_temp_detach.dyn_g++_64_pic_none
		pc_terminate.dyn_g++_64_pic_none
		pc_terminate_stopped.dyn_g++_64_pic_none
		pc_thread.dyn_g++_64_pic_none
		pc_thread_cont.dyn_g++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
