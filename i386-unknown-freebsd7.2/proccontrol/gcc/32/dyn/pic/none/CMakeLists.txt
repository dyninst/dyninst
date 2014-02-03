# CMakeLists for proccontrol/gcc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"c"
	"gnu_cc 32 dyn"
	MUTATEEgnucc32dync)

IF (MUTATEEgnucc32dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-freebsd7.2/srclists.cmake)
add_executable (pc_addlibrary.dyn_gcc_32_pic_none ${SOURCE_LIST_46})
target_link_libraries (pc_addlibrary.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_gcc_32_pic_none ${SOURCE_LIST_37})
target_link_libraries (pc_breakpoint.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_gcc_32_pic_none ${SOURCE_LIST_11})
target_link_libraries (pc_detach.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_gcc_32_pic_none ${SOURCE_LIST_44})
target_link_libraries (pc_groups.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_gcc_32_pic_none ${SOURCE_LIST_72})
target_link_libraries (pc_irpc.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_gcc_32_pic_none ${SOURCE_LIST_73})
target_link_libraries (pc_launch.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_gcc_32_pic_none ${SOURCE_LIST_32})
target_link_libraries (pc_library.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_gcc_32_pic_none ${SOURCE_LIST_34})
target_link_libraries (pc_singlestep.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_gcc_32_pic_none ${SOURCE_LIST_9})
target_link_libraries (pc_stat.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_gcc_32_pic_none ${SOURCE_LIST_5})
target_link_libraries (pc_temp_detach.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_gcc_32_pic_none ${SOURCE_LIST_30})
target_link_libraries (pc_terminate.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_gcc_32_pic_none ${SOURCE_LIST_55})
target_link_libraries (pc_terminate_stopped.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_gcc_32_pic_none ${SOURCE_LIST_43})
target_link_libraries (pc_thread.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_gcc_32_pic_none ${SOURCE_LIST_21})
target_link_libraries (pc_thread_cont.dyn_gcc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_gcc_32_pic_none
		pc_breakpoint.dyn_gcc_32_pic_none
		pc_detach.dyn_gcc_32_pic_none
		pc_groups.dyn_gcc_32_pic_none
		pc_irpc.dyn_gcc_32_pic_none
		pc_launch.dyn_gcc_32_pic_none
		pc_library.dyn_gcc_32_pic_none
		pc_singlestep.dyn_gcc_32_pic_none
		pc_stat.dyn_gcc_32_pic_none
		pc_temp_detach.dyn_gcc_32_pic_none
		pc_terminate.dyn_gcc_32_pic_none
		pc_terminate_stopped.dyn_gcc_32_pic_none
		pc_thread.dyn_gcc_32_pic_none
		pc_thread_cont.dyn_gcc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cc not present")
ENDIF()
