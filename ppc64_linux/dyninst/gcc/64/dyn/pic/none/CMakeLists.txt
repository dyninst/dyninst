# CMakeLists for dyninst/gcc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -m64  -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m64 -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}  -m64  -fPIC"
	"${MUTATEE_LDFLAGS_GNU} -m64 -Wl,-export-dynamic"
	"c"
	"gnu_cc 64 dyn"
	MUTATEEgnucc64dync)

IF (MUTATEEgnucc64dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_linux/srclists.cmake)
add_executable (dyninst_group_testdyn_gcc_64_pic_none ${SOURCE_LIST_14})
target_link_libraries (dyninst_group_testdyn_gcc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callbackdyn_gcc_64_pic_none ${SOURCE_LIST_54})
target_link_libraries (init_fini_callbackdyn_gcc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_12dyn_gcc_64_pic_none ${SOURCE_LIST_65})
add_executable (test1_14dyn_gcc_64_pic_none ${SOURCE_LIST_43})
add_executable (test1_19dyn_gcc_64_pic_none ${SOURCE_LIST_22})
add_executable (test1_29dyn_gcc_64_pic_none ${SOURCE_LIST_4})
add_executable (test1_40dyn_gcc_64_pic_none ${SOURCE_LIST_36})
add_executable (test1_41dyn_gcc_64_pic_none ${SOURCE_LIST_7})
add_executable (test2_14dyn_gcc_64_pic_none ${SOURCE_LIST_55})
add_executable (test2_6dyn_gcc_64_pic_none ${SOURCE_LIST_1})
target_link_libraries (test2_6dyn_gcc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test2_8dyn_gcc_64_pic_none ${SOURCE_LIST_2})
add_executable (test3_1dyn_gcc_64_pic_none ${SOURCE_LIST_30})
add_executable (test3_2dyn_gcc_64_pic_none ${SOURCE_LIST_35})
add_executable (test3_3dyn_gcc_64_pic_none ${SOURCE_LIST_50})
add_executable (test3_4dyn_gcc_64_pic_none ${SOURCE_LIST_58})
add_executable (test3_5dyn_gcc_64_pic_none ${SOURCE_LIST_45})
add_executable (test3_6dyn_gcc_64_pic_none ${SOURCE_LIST_48})
add_executable (test3_7dyn_gcc_64_pic_none ${SOURCE_LIST_26})
add_executable (test4_1dyn_gcc_64_pic_none ${SOURCE_LIST_3})
add_executable (test4_2dyn_gcc_64_pic_none ${SOURCE_LIST_64})
add_executable (test4_3dyn_gcc_64_pic_none ${SOURCE_LIST_60})
add_executable (test4_3bdyn_gcc_64_pic_none ${SOURCE_LIST_66})
add_executable (test4_4dyn_gcc_64_pic_none ${SOURCE_LIST_38})
add_executable (test4_4bdyn_gcc_64_pic_none ${SOURCE_LIST_56})
add_executable (test_callback_1dyn_gcc_64_pic_none ${SOURCE_LIST_15})
add_executable (test_callback_2dyn_gcc_64_pic_none ${SOURCE_LIST_28})
add_executable (test_fork_10dyn_gcc_64_pic_none ${SOURCE_LIST_46})
add_executable (test_fork_11dyn_gcc_64_pic_none ${SOURCE_LIST_18})
add_executable (test_fork_12dyn_gcc_64_pic_none ${SOURCE_LIST_25})
add_executable (test_fork_13dyn_gcc_64_pic_none ${SOURCE_LIST_6})
add_executable (test_fork_14dyn_gcc_64_pic_none ${SOURCE_LIST_19})
add_executable (test_fork_5dyn_gcc_64_pic_none ${SOURCE_LIST_61})
add_executable (test_fork_6dyn_gcc_64_pic_none ${SOURCE_LIST_47})
add_executable (test_fork_7dyn_gcc_64_pic_none ${SOURCE_LIST_23})
add_executable (test_fork_8dyn_gcc_64_pic_none ${SOURCE_LIST_16})
add_executable (test_fork_9dyn_gcc_64_pic_none ${SOURCE_LIST_37})
add_executable (test_snip_removedyn_gcc_64_pic_none ${SOURCE_LIST_49})
add_executable (test_stack_1dyn_gcc_64_pic_none ${SOURCE_LIST_42})
add_executable (test_stack_2dyn_gcc_64_pic_none ${SOURCE_LIST_57})
add_executable (test_stack_3dyn_gcc_64_pic_none ${SOURCE_LIST_52})
add_executable (test_stack_4dyn_gcc_64_pic_none ${SOURCE_LIST_62})
add_executable (test_thread_1dyn_gcc_64_pic_none ${SOURCE_LIST_32})
target_link_libraries (test_thread_1dyn_gcc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2dyn_gcc_64_pic_none ${SOURCE_LIST_33})
target_link_libraries (test_thread_2dyn_gcc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3dyn_gcc_64_pic_none ${SOURCE_LIST_21})
target_link_libraries (test_thread_3dyn_gcc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5dyn_gcc_64_pic_none ${SOURCE_LIST_13})
target_link_libraries (test_thread_5dyn_gcc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6dyn_gcc_64_pic_none ${SOURCE_LIST_59})
target_link_libraries (test_thread_6dyn_gcc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7dyn_gcc_64_pic_none ${SOURCE_LIST_63})
target_link_libraries (test_thread_7dyn_gcc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8dyn_gcc_64_pic_none ${SOURCE_LIST_8})
target_link_libraries (test_thread_8dyn_gcc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_group_testdyn_gcc_64_pic_none
		init_fini_callbackdyn_gcc_64_pic_none
		test1_12dyn_gcc_64_pic_none
		test1_14dyn_gcc_64_pic_none
		test1_19dyn_gcc_64_pic_none
		test1_29dyn_gcc_64_pic_none
		test1_40dyn_gcc_64_pic_none
		test1_41dyn_gcc_64_pic_none
		test2_14dyn_gcc_64_pic_none
		test2_6dyn_gcc_64_pic_none
		test2_8dyn_gcc_64_pic_none
		test3_1dyn_gcc_64_pic_none
		test3_2dyn_gcc_64_pic_none
		test3_3dyn_gcc_64_pic_none
		test3_4dyn_gcc_64_pic_none
		test3_5dyn_gcc_64_pic_none
		test3_6dyn_gcc_64_pic_none
		test3_7dyn_gcc_64_pic_none
		test4_1dyn_gcc_64_pic_none
		test4_2dyn_gcc_64_pic_none
		test4_3dyn_gcc_64_pic_none
		test4_3bdyn_gcc_64_pic_none
		test4_4dyn_gcc_64_pic_none
		test4_4bdyn_gcc_64_pic_none
		test_callback_1dyn_gcc_64_pic_none
		test_callback_2dyn_gcc_64_pic_none
		test_fork_10dyn_gcc_64_pic_none
		test_fork_11dyn_gcc_64_pic_none
		test_fork_12dyn_gcc_64_pic_none
		test_fork_13dyn_gcc_64_pic_none
		test_fork_14dyn_gcc_64_pic_none
		test_fork_5dyn_gcc_64_pic_none
		test_fork_6dyn_gcc_64_pic_none
		test_fork_7dyn_gcc_64_pic_none
		test_fork_8dyn_gcc_64_pic_none
		test_fork_9dyn_gcc_64_pic_none
		test_snip_removedyn_gcc_64_pic_none
		test_stack_1dyn_gcc_64_pic_none
		test_stack_2dyn_gcc_64_pic_none
		test_stack_3dyn_gcc_64_pic_none
		test_stack_4dyn_gcc_64_pic_none
		test_thread_1dyn_gcc_64_pic_none
		test_thread_2dyn_gcc_64_pic_none
		test_thread_3dyn_gcc_64_pic_none
		test_thread_5dyn_gcc_64_pic_none
		test_thread_6dyn_gcc_64_pic_none
		test_thread_7dyn_gcc_64_pic_none
		test_thread_8dyn_gcc_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cc not present")
ENDIF()
