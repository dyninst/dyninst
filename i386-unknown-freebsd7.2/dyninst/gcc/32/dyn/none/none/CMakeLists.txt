# CMakeLists for dyninst/gcc/32/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"c"
	"gnu_cc 32 dyn"
	MUTATEEgnucc32dync)

IF (MUTATEEgnucc32dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-freebsd7.2/srclists.cmake)
add_executable (dyninst_group_testdyn_gcc_32_none_none ${SOURCE_LIST_26})
add_executable (init_fini_callbackdyn_gcc_32_none_none ${SOURCE_LIST_59})
add_executable (test1_12dyn_gcc_32_none_none ${SOURCE_LIST_70})
add_executable (test1_14dyn_gcc_32_none_none ${SOURCE_LIST_48})
add_executable (test1_19dyn_gcc_32_none_none ${SOURCE_LIST_24})
add_executable (test1_29dyn_gcc_32_none_none ${SOURCE_LIST_4})
add_executable (test1_35dyn_gcc_32_none_none ${SOURCE_LIST_66})
add_executable (test1_40dyn_gcc_32_none_none ${SOURCE_LIST_40})
add_executable (test1_41dyn_gcc_32_none_none ${SOURCE_LIST_6})
add_executable (test2_14dyn_gcc_32_none_none ${SOURCE_LIST_60})
add_executable (test2_6dyn_gcc_32_none_none ${SOURCE_LIST_1})
add_executable (test2_8dyn_gcc_32_none_none ${SOURCE_LIST_2})
add_executable (test3_1dyn_gcc_32_none_none ${SOURCE_LIST_33})
add_executable (test3_2dyn_gcc_32_none_none ${SOURCE_LIST_39})
add_executable (test3_3dyn_gcc_32_none_none ${SOURCE_LIST_54})
add_executable (test3_4dyn_gcc_32_none_none ${SOURCE_LIST_63})
add_executable (test3_5dyn_gcc_32_none_none ${SOURCE_LIST_50})
add_executable (test3_6dyn_gcc_32_none_none ${SOURCE_LIST_0})
add_executable (test3_7dyn_gcc_32_none_none ${SOURCE_LIST_29})
add_executable (test4_1dyn_gcc_32_none_none ${SOURCE_LIST_3})
add_executable (test4_2dyn_gcc_32_none_none ${SOURCE_LIST_58})
add_executable (test4_3dyn_gcc_32_none_none ${SOURCE_LIST_65})
add_executable (test4_3bdyn_gcc_32_none_none ${SOURCE_LIST_71})
add_executable (test4_4dyn_gcc_32_none_none ${SOURCE_LIST_42})
add_executable (test4_4bdyn_gcc_32_none_none ${SOURCE_LIST_61})
add_executable (test_callback_1dyn_gcc_32_none_none ${SOURCE_LIST_15})
add_executable (test_callback_2dyn_gcc_32_none_none ${SOURCE_LIST_31})
add_executable (test_fork_10dyn_gcc_32_none_none ${SOURCE_LIST_51})
add_executable (test_fork_11dyn_gcc_32_none_none ${SOURCE_LIST_18})
add_executable (test_fork_12dyn_gcc_32_none_none ${SOURCE_LIST_28})
add_executable (test_fork_13dyn_gcc_32_none_none ${SOURCE_LIST_10})
add_executable (test_fork_14dyn_gcc_32_none_none ${SOURCE_LIST_20})
add_executable (test_fork_5dyn_gcc_32_none_none ${SOURCE_LIST_27})
add_executable (test_fork_6dyn_gcc_32_none_none ${SOURCE_LIST_52})
add_executable (test_fork_7dyn_gcc_32_none_none ${SOURCE_LIST_25})
add_executable (test_fork_8dyn_gcc_32_none_none ${SOURCE_LIST_16})
add_executable (test_fork_9dyn_gcc_32_none_none ${SOURCE_LIST_41})
add_executable (test_mem_1dyn_gcc_32_none_none ${SOURCE_LIST_56})
add_executable (test_mem_2dyn_gcc_32_none_none ${SOURCE_LIST_45})
add_executable (test_mem_3dyn_gcc_32_none_none ${SOURCE_LIST_49})
add_executable (test_mem_4dyn_gcc_32_none_none ${SOURCE_LIST_8})
add_executable (test_mem_5dyn_gcc_32_none_none ${SOURCE_LIST_19})
add_executable (test_mem_6dyn_gcc_32_none_none ${SOURCE_LIST_38})
add_executable (test_mem_7dyn_gcc_32_none_none ${SOURCE_LIST_12})
add_executable (test_mem_8dyn_gcc_32_none_none ${SOURCE_LIST_22})
add_executable (test_snip_removedyn_gcc_32_none_none ${SOURCE_LIST_53})
add_executable (test_stack_1dyn_gcc_32_none_none ${SOURCE_LIST_47})
add_executable (test_stack_2dyn_gcc_32_none_none ${SOURCE_LIST_62})
add_executable (test_stack_3dyn_gcc_32_none_none ${SOURCE_LIST_57})
add_executable (test_stack_4dyn_gcc_32_none_none ${SOURCE_LIST_67})
add_executable (test_thread_1dyn_gcc_32_none_none ${SOURCE_LIST_35})
target_link_libraries (test_thread_1dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2dyn_gcc_32_none_none ${SOURCE_LIST_36})
target_link_libraries (test_thread_2dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3dyn_gcc_32_none_none ${SOURCE_LIST_13})
target_link_libraries (test_thread_3dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5dyn_gcc_32_none_none ${SOURCE_LIST_14})
target_link_libraries (test_thread_5dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6dyn_gcc_32_none_none ${SOURCE_LIST_64})
target_link_libraries (test_thread_6dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7dyn_gcc_32_none_none ${SOURCE_LIST_68})
target_link_libraries (test_thread_7dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8dyn_gcc_32_none_none ${SOURCE_LIST_7})
target_link_libraries (test_thread_8dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_group_testdyn_gcc_32_none_none
		init_fini_callbackdyn_gcc_32_none_none
		test1_12dyn_gcc_32_none_none
		test1_14dyn_gcc_32_none_none
		test1_19dyn_gcc_32_none_none
		test1_29dyn_gcc_32_none_none
		test1_35dyn_gcc_32_none_none
		test1_40dyn_gcc_32_none_none
		test1_41dyn_gcc_32_none_none
		test2_14dyn_gcc_32_none_none
		test2_6dyn_gcc_32_none_none
		test2_8dyn_gcc_32_none_none
		test3_1dyn_gcc_32_none_none
		test3_2dyn_gcc_32_none_none
		test3_3dyn_gcc_32_none_none
		test3_4dyn_gcc_32_none_none
		test3_5dyn_gcc_32_none_none
		test3_6dyn_gcc_32_none_none
		test3_7dyn_gcc_32_none_none
		test4_1dyn_gcc_32_none_none
		test4_2dyn_gcc_32_none_none
		test4_3dyn_gcc_32_none_none
		test4_3bdyn_gcc_32_none_none
		test4_4dyn_gcc_32_none_none
		test4_4bdyn_gcc_32_none_none
		test_callback_1dyn_gcc_32_none_none
		test_callback_2dyn_gcc_32_none_none
		test_fork_10dyn_gcc_32_none_none
		test_fork_11dyn_gcc_32_none_none
		test_fork_12dyn_gcc_32_none_none
		test_fork_13dyn_gcc_32_none_none
		test_fork_14dyn_gcc_32_none_none
		test_fork_5dyn_gcc_32_none_none
		test_fork_6dyn_gcc_32_none_none
		test_fork_7dyn_gcc_32_none_none
		test_fork_8dyn_gcc_32_none_none
		test_fork_9dyn_gcc_32_none_none
		test_mem_1dyn_gcc_32_none_none
		test_mem_2dyn_gcc_32_none_none
		test_mem_3dyn_gcc_32_none_none
		test_mem_4dyn_gcc_32_none_none
		test_mem_5dyn_gcc_32_none_none
		test_mem_6dyn_gcc_32_none_none
		test_mem_7dyn_gcc_32_none_none
		test_mem_8dyn_gcc_32_none_none
		test_snip_removedyn_gcc_32_none_none
		test_stack_1dyn_gcc_32_none_none
		test_stack_2dyn_gcc_32_none_none
		test_stack_3dyn_gcc_32_none_none
		test_stack_4dyn_gcc_32_none_none
		test_thread_1dyn_gcc_32_none_none
		test_thread_2dyn_gcc_32_none_none
		test_thread_3dyn_gcc_32_none_none
		test_thread_5dyn_gcc_32_none_none
		test_thread_6dyn_gcc_32_none_none
		test_thread_7dyn_gcc_32_none_none
		test_thread_8dyn_gcc_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cc not present")
ENDIF()
