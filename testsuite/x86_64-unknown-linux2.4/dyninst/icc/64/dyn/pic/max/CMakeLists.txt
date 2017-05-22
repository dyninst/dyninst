# CMakeLists for dyninst/icc/64/dyn/pic/max
set (CMAKE_C_FLAGS "${TEST_DEFINES} -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}   -O3 -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILE_OBJECT "${M_intel_cc} -c <FLAGS> -o <OBJECT> -c <SOURCE>")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (amd64_7_arg_call.dyn_icc_64_pic_max ${SOURCE_LIST_47})
add_executable (dyninst_group_test.dyn_icc_64_pic_max ${SOURCE_LIST_36})
target_link_libraries (dyninst_group_test.dyn_icc_64_pic_max dl)
add_executable (init_fini_callback.dyn_icc_64_pic_max ${SOURCE_LIST_63})
target_link_libraries (init_fini_callback.dyn_icc_64_pic_max dl)
add_executable (test1_12.dyn_icc_64_pic_max ${SOURCE_LIST_76})
add_executable (test1_14.dyn_icc_64_pic_max ${SOURCE_LIST_53})
add_executable (test1_19.dyn_icc_64_pic_max ${SOURCE_LIST_25})
add_executable (test1_29.dyn_icc_64_pic_max ${SOURCE_LIST_4})
add_executable (test1_35.dyn_icc_64_pic_max ${SOURCE_LIST_44})
add_executable (test1_40.dyn_icc_64_pic_max ${SOURCE_LIST_41})
add_executable (test1_41.dyn_icc_64_pic_max ${SOURCE_LIST_8})
add_executable (test2_14.dyn_icc_64_pic_max ${SOURCE_LIST_64})
add_executable (test2_6.dyn_icc_64_pic_max ${SOURCE_LIST_1})
target_link_libraries (test2_6.dyn_icc_64_pic_max dl)
add_executable (test2_8.dyn_icc_64_pic_max ${SOURCE_LIST_2})
add_executable (test3_1.dyn_icc_64_pic_max ${SOURCE_LIST_34})
add_executable (test3_2.dyn_icc_64_pic_max ${SOURCE_LIST_40})
add_executable (test3_3.dyn_icc_64_pic_max ${SOURCE_LIST_59})
add_executable (test3_4.dyn_icc_64_pic_max ${SOURCE_LIST_67})
add_executable (test3_5.dyn_icc_64_pic_max ${SOURCE_LIST_55})
add_executable (test3_6.dyn_icc_64_pic_max ${SOURCE_LIST_57})
add_executable (test3_7.dyn_icc_64_pic_max ${SOURCE_LIST_30})
add_executable (test4_1.dyn_icc_64_pic_max ${SOURCE_LIST_3})
add_executable (test4_2.dyn_icc_64_pic_max ${SOURCE_LIST_75})
add_executable (test4_3.dyn_icc_64_pic_max ${SOURCE_LIST_70})
add_executable (test4_3b.dyn_icc_64_pic_max ${SOURCE_LIST_77})
add_executable (test4_4.dyn_icc_64_pic_max ${SOURCE_LIST_45})
add_executable (test4_4b.dyn_icc_64_pic_max ${SOURCE_LIST_65})
add_executable (test_callback_1.dyn_icc_64_pic_max ${SOURCE_LIST_18})
add_executable (test_callback_2.dyn_icc_64_pic_max ${SOURCE_LIST_32})
add_executable (test_fork_10.dyn_icc_64_pic_max ${SOURCE_LIST_56})
add_executable (test_fork_11.dyn_icc_64_pic_max ${SOURCE_LIST_21})
add_executable (test_fork_12.dyn_icc_64_pic_max ${SOURCE_LIST_29})
add_executable (test_fork_13.dyn_icc_64_pic_max ${SOURCE_LIST_14})
add_executable (test_fork_5.dyn_icc_64_pic_max ${SOURCE_LIST_71})
add_executable (test_fork_6.dyn_icc_64_pic_max ${SOURCE_LIST_23})
add_executable (test_fork_7.dyn_icc_64_pic_max ${SOURCE_LIST_26})
add_executable (test_fork_8.dyn_icc_64_pic_max ${SOURCE_LIST_19})
add_executable (test_fork_9.dyn_icc_64_pic_max ${SOURCE_LIST_43})
add_executable (test_mem_1.dyn_icc_64_pic_max ${SOURCE_LIST_42})
add_executable (test_mem_2.dyn_icc_64_pic_max ${SOURCE_LIST_7})
add_executable (test_mem_3.dyn_icc_64_pic_max ${SOURCE_LIST_10})
add_executable (test_mem_4.dyn_icc_64_pic_max ${SOURCE_LIST_74})
add_executable (test_mem_5.dyn_icc_64_pic_max ${SOURCE_LIST_61})
add_executable (test_mem_6.dyn_icc_64_pic_max ${SOURCE_LIST_28})
add_executable (test_mem_7.dyn_icc_64_pic_max ${SOURCE_LIST_22})
add_executable (test_mem_8.dyn_icc_64_pic_max ${SOURCE_LIST_38})
add_executable (test_snip_remove.dyn_icc_64_pic_max ${SOURCE_LIST_58})
add_executable (test_stack_1.dyn_icc_64_pic_max ${SOURCE_LIST_52})
add_executable (test_stack_2.dyn_icc_64_pic_max ${SOURCE_LIST_66})
add_executable (test_stack_3.dyn_icc_64_pic_max ${SOURCE_LIST_48})
add_executable (test_stack_4.dyn_icc_64_pic_max ${SOURCE_LIST_72})
add_executable (test_thread_1.dyn_icc_64_pic_max ${SOURCE_LIST_37})
target_link_libraries (test_thread_1.dyn_icc_64_pic_max dl pthread)
add_executable (test_thread_2.dyn_icc_64_pic_max ${SOURCE_LIST_12})
target_link_libraries (test_thread_2.dyn_icc_64_pic_max dl pthread)
add_executable (test_thread_3.dyn_icc_64_pic_max ${SOURCE_LIST_17})
target_link_libraries (test_thread_3.dyn_icc_64_pic_max dl pthread)
add_executable (test_thread_5.dyn_icc_64_pic_max ${SOURCE_LIST_16})
target_link_libraries (test_thread_5.dyn_icc_64_pic_max dl pthread)
add_executable (test_thread_6.dyn_icc_64_pic_max ${SOURCE_LIST_68})
target_link_libraries (test_thread_6.dyn_icc_64_pic_max pthread)
add_executable (test_thread_7.dyn_icc_64_pic_max ${SOURCE_LIST_73})
target_link_libraries (test_thread_7.dyn_icc_64_pic_max pthread)
add_executable (test_thread_8.dyn_icc_64_pic_max ${SOURCE_LIST_11})
target_link_libraries (test_thread_8.dyn_icc_64_pic_max pthread)


INSTALL (TARGETS
		amd64_7_arg_call.dyn_icc_64_pic_max
		dyninst_group_test.dyn_icc_64_pic_max
		init_fini_callback.dyn_icc_64_pic_max
		test1_12.dyn_icc_64_pic_max
		test1_14.dyn_icc_64_pic_max
		test1_19.dyn_icc_64_pic_max
		test1_29.dyn_icc_64_pic_max
		test1_35.dyn_icc_64_pic_max
		test1_40.dyn_icc_64_pic_max
		test1_41.dyn_icc_64_pic_max
		test2_14.dyn_icc_64_pic_max
		test2_6.dyn_icc_64_pic_max
		test2_8.dyn_icc_64_pic_max
		test3_1.dyn_icc_64_pic_max
		test3_2.dyn_icc_64_pic_max
		test3_3.dyn_icc_64_pic_max
		test3_4.dyn_icc_64_pic_max
		test3_5.dyn_icc_64_pic_max
		test3_6.dyn_icc_64_pic_max
		test3_7.dyn_icc_64_pic_max
		test4_1.dyn_icc_64_pic_max
		test4_2.dyn_icc_64_pic_max
		test4_3.dyn_icc_64_pic_max
		test4_3b.dyn_icc_64_pic_max
		test4_4.dyn_icc_64_pic_max
		test4_4b.dyn_icc_64_pic_max
		test_callback_1.dyn_icc_64_pic_max
		test_callback_2.dyn_icc_64_pic_max
		test_fork_10.dyn_icc_64_pic_max
		test_fork_11.dyn_icc_64_pic_max
		test_fork_12.dyn_icc_64_pic_max
		test_fork_13.dyn_icc_64_pic_max
		test_fork_5.dyn_icc_64_pic_max
		test_fork_6.dyn_icc_64_pic_max
		test_fork_7.dyn_icc_64_pic_max
		test_fork_8.dyn_icc_64_pic_max
		test_fork_9.dyn_icc_64_pic_max
		test_mem_1.dyn_icc_64_pic_max
		test_mem_2.dyn_icc_64_pic_max
		test_mem_3.dyn_icc_64_pic_max
		test_mem_4.dyn_icc_64_pic_max
		test_mem_5.dyn_icc_64_pic_max
		test_mem_6.dyn_icc_64_pic_max
		test_mem_7.dyn_icc_64_pic_max
		test_mem_8.dyn_icc_64_pic_max
		test_snip_remove.dyn_icc_64_pic_max
		test_stack_1.dyn_icc_64_pic_max
		test_stack_2.dyn_icc_64_pic_max
		test_stack_3.dyn_icc_64_pic_max
		test_stack_4.dyn_icc_64_pic_max
		test_thread_1.dyn_icc_64_pic_max
		test_thread_2.dyn_icc_64_pic_max
		test_thread_3.dyn_icc_64_pic_max
		test_thread_5.dyn_icc_64_pic_max
		test_thread_6.dyn_icc_64_pic_max
		test_thread_7.dyn_icc_64_pic_max
		test_thread_8.dyn_icc_64_pic_max
	DESTINATION testsuite)
