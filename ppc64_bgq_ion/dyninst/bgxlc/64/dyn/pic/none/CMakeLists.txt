# CMakeLists for dyninst/bgxlc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}   -qpic")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_bg_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
include (${PROJECT_SRC_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_group_test.dyn_bgxlc_64_pic_none ${SOURCE_LIST_44})
target_link_libraries (dyninst_group_test.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_bgxlc_64_pic_none ${SOURCE_LIST_46})
target_link_libraries (init_fini_callback.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_bgxlc_64_pic_none ${SOURCE_LIST_57})
add_executable (test1_14.dyn_bgxlc_64_pic_none ${SOURCE_LIST_38})
add_executable (test1_19.dyn_bgxlc_64_pic_none ${SOURCE_LIST_20})
add_executable (test1_29.dyn_bgxlc_64_pic_none ${SOURCE_LIST_9})
add_executable (test1_41.dyn_bgxlc_64_pic_none ${SOURCE_LIST_7})
add_executable (test2_14.dyn_bgxlc_64_pic_none ${SOURCE_LIST_47})
add_executable (test2_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_1})
target_link_libraries (test2_6.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_bgxlc_64_pic_none ${SOURCE_LIST_2})
add_executable (test3_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_26})
add_executable (test3_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_31})
add_executable (test3_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_42})
add_executable (test3_4.dyn_bgxlc_64_pic_none ${SOURCE_LIST_50})
add_executable (test3_5.dyn_bgxlc_64_pic_none ${SOURCE_LIST_39})
add_executable (test3_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_0})
add_executable (test3_7.dyn_bgxlc_64_pic_none ${SOURCE_LIST_5})
add_executable (test4_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_3})
add_executable (test4_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_56})
add_executable (test4_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_52})
add_executable (test4_3b.dyn_bgxlc_64_pic_none ${SOURCE_LIST_58})
add_executable (test4_4.dyn_bgxlc_64_pic_none ${SOURCE_LIST_33})
add_executable (test4_4b.dyn_bgxlc_64_pic_none ${SOURCE_LIST_10})
add_executable (test_callback_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_14})
add_executable (test_callback_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_24})
add_executable (test_fork_10.dyn_bgxlc_64_pic_none ${SOURCE_LIST_40})
add_executable (test_fork_11.dyn_bgxlc_64_pic_none ${SOURCE_LIST_17})
add_executable (test_fork_12.dyn_bgxlc_64_pic_none ${SOURCE_LIST_22})
add_executable (test_fork_13.dyn_bgxlc_64_pic_none ${SOURCE_LIST_6})
add_executable (test_fork_5.dyn_bgxlc_64_pic_none ${SOURCE_LIST_53})
add_executable (test_fork_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_18})
add_executable (test_fork_7.dyn_bgxlc_64_pic_none ${SOURCE_LIST_21})
add_executable (test_fork_8.dyn_bgxlc_64_pic_none ${SOURCE_LIST_15})
add_executable (test_fork_9.dyn_bgxlc_64_pic_none ${SOURCE_LIST_32})
add_executable (test_snip_remove.dyn_bgxlc_64_pic_none ${SOURCE_LIST_41})
add_executable (test_stack_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_37})
add_executable (test_stack_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_49})
add_executable (test_stack_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_45})
add_executable (test_stack_4.dyn_bgxlc_64_pic_none ${SOURCE_LIST_19})
add_executable (test_thread_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_28})
target_link_libraries (test_thread_1.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_29})
target_link_libraries (test_thread_2.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_13})
target_link_libraries (test_thread_3.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_bgxlc_64_pic_none ${SOURCE_LIST_12})
target_link_libraries (test_thread_5.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_51})
target_link_libraries (test_thread_6.dyn_bgxlc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_bgxlc_64_pic_none ${SOURCE_LIST_55})
target_link_libraries (test_thread_7.dyn_bgxlc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_bgxlc_64_pic_none ${SOURCE_LIST_8})
target_link_libraries (test_thread_8.dyn_bgxlc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_group_test.dyn_bgxlc_64_pic_none
		init_fini_callback.dyn_bgxlc_64_pic_none
		test1_12.dyn_bgxlc_64_pic_none
		test1_14.dyn_bgxlc_64_pic_none
		test1_19.dyn_bgxlc_64_pic_none
		test1_29.dyn_bgxlc_64_pic_none
		test1_41.dyn_bgxlc_64_pic_none
		test2_14.dyn_bgxlc_64_pic_none
		test2_6.dyn_bgxlc_64_pic_none
		test2_8.dyn_bgxlc_64_pic_none
		test3_1.dyn_bgxlc_64_pic_none
		test3_2.dyn_bgxlc_64_pic_none
		test3_3.dyn_bgxlc_64_pic_none
		test3_4.dyn_bgxlc_64_pic_none
		test3_5.dyn_bgxlc_64_pic_none
		test3_6.dyn_bgxlc_64_pic_none
		test3_7.dyn_bgxlc_64_pic_none
		test4_1.dyn_bgxlc_64_pic_none
		test4_2.dyn_bgxlc_64_pic_none
		test4_3.dyn_bgxlc_64_pic_none
		test4_3b.dyn_bgxlc_64_pic_none
		test4_4.dyn_bgxlc_64_pic_none
		test4_4b.dyn_bgxlc_64_pic_none
		test_callback_1.dyn_bgxlc_64_pic_none
		test_callback_2.dyn_bgxlc_64_pic_none
		test_fork_10.dyn_bgxlc_64_pic_none
		test_fork_11.dyn_bgxlc_64_pic_none
		test_fork_12.dyn_bgxlc_64_pic_none
		test_fork_13.dyn_bgxlc_64_pic_none
		test_fork_5.dyn_bgxlc_64_pic_none
		test_fork_6.dyn_bgxlc_64_pic_none
		test_fork_7.dyn_bgxlc_64_pic_none
		test_fork_8.dyn_bgxlc_64_pic_none
		test_fork_9.dyn_bgxlc_64_pic_none
		test_snip_remove.dyn_bgxlc_64_pic_none
		test_stack_1.dyn_bgxlc_64_pic_none
		test_stack_2.dyn_bgxlc_64_pic_none
		test_stack_3.dyn_bgxlc_64_pic_none
		test_stack_4.dyn_bgxlc_64_pic_none
		test_thread_1.dyn_bgxlc_64_pic_none
		test_thread_2.dyn_bgxlc_64_pic_none
		test_thread_3.dyn_bgxlc_64_pic_none
		test_thread_5.dyn_bgxlc_64_pic_none
		test_thread_6.dyn_bgxlc_64_pic_none
		test_thread_7.dyn_bgxlc_64_pic_none
		test_thread_8.dyn_bgxlc_64_pic_none
	DESTINATION ${INSTALL_DIR})
