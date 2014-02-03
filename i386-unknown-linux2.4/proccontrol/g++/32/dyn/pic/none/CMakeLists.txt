# CMakeLists for proccontrol/g++/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
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
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"cxx"
	"gnu_cxx 32 dyn"
	MUTATEEgnucxx32dyncxx)

IF (MUTATEEgnucxx32dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (pc_addlibrary.dyn_g++_32_pic_none ${SOURCE_LIST_51})
target_link_libraries (pc_addlibrary.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_g++_32_pic_none ${SOURCE_LIST_68})
target_link_libraries (pc_breakpoint.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_g++_32_pic_none ${SOURCE_LIST_60})
target_link_libraries (pc_detach.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_exec_targ.dyn_g++_32_pic_none ${SOURCE_LIST_11})
add_executable (pc_fork.dyn_g++_32_pic_none ${SOURCE_LIST_80})
target_link_libraries (pc_fork.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_fork_exec.dyn_g++_32_pic_none ${SOURCE_LIST_99})
target_link_libraries (pc_fork_exec.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_g++_32_pic_none ${SOURCE_LIST_93})
target_link_libraries (pc_groups.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_hw_breakpoint.dyn_g++_32_pic_none ${SOURCE_LIST_4})
target_link_libraries (pc_hw_breakpoint.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_g++_32_pic_none ${SOURCE_LIST_79})
target_link_libraries (pc_irpc.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_g++_32_pic_none ${SOURCE_LIST_21})
target_link_libraries (pc_launch.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_g++_32_pic_none ${SOURCE_LIST_14})
target_link_libraries (pc_library.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_g++_32_pic_none ${SOURCE_LIST_44})
target_link_libraries (pc_singlestep.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_g++_32_pic_none ${SOURCE_LIST_84})
target_link_libraries (pc_stat.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_g++_32_pic_none ${SOURCE_LIST_3})
target_link_libraries (pc_temp_detach.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_g++_32_pic_none ${SOURCE_LIST_42})
target_link_libraries (pc_terminate.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_g++_32_pic_none ${SOURCE_LIST_97})
target_link_libraries (pc_terminate_stopped.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_g++_32_pic_none ${SOURCE_LIST_17})
target_link_libraries (pc_thread.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_g++_32_pic_none ${SOURCE_LIST_8})
target_link_libraries (pc_thread_cont.dyn_g++_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_g++_32_pic_none
		pc_breakpoint.dyn_g++_32_pic_none
		pc_detach.dyn_g++_32_pic_none
		pc_exec_targ.dyn_g++_32_pic_none
		pc_fork.dyn_g++_32_pic_none
		pc_fork_exec.dyn_g++_32_pic_none
		pc_groups.dyn_g++_32_pic_none
		pc_hw_breakpoint.dyn_g++_32_pic_none
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
