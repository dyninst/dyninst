# CMakeLists for proccontrol/g++/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"cxx"
	"gnu_cxx 64 dyn"
	MUTATEEgnucxx64dyncxx)

IF (MUTATEEgnucxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/amd64-unknown-freebsd7.2/srclists.cmake)
add_executable (pc_addlibrary.dyn_g++_64_pic_none ${SOURCE_LIST_48})
target_link_libraries (pc_addlibrary.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_g++_64_pic_none ${SOURCE_LIST_37})
target_link_libraries (pc_breakpoint.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_g++_64_pic_none ${SOURCE_LIST_15})
target_link_libraries (pc_detach.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_g++_64_pic_none ${SOURCE_LIST_47})
target_link_libraries (pc_groups.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_g++_64_pic_none ${SOURCE_LIST_73})
target_link_libraries (pc_irpc.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_g++_64_pic_none ${SOURCE_LIST_74})
target_link_libraries (pc_launch.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_g++_64_pic_none ${SOURCE_LIST_32})
target_link_libraries (pc_library.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_g++_64_pic_none ${SOURCE_LIST_34})
target_link_libraries (pc_singlestep.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_g++_64_pic_none ${SOURCE_LIST_13})
target_link_libraries (pc_stat.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_g++_64_pic_none ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detach.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_g++_64_pic_none ${SOURCE_LIST_30})
target_link_libraries (pc_terminate.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_g++_64_pic_none ${SOURCE_LIST_56})
target_link_libraries (pc_terminate_stopped.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_g++_64_pic_none ${SOURCE_LIST_44})
target_link_libraries (pc_thread.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_g++_64_pic_none ${SOURCE_LIST_24})
target_link_libraries (pc_thread_cont.dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_g++_64_pic_none
		pc_breakpoint.dyn_g++_64_pic_none
		pc_detach.dyn_g++_64_pic_none
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

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
