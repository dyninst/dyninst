# CMakeLists for dyninst/g++/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
include (../../../../../../srclists.cmake)
add_executable (dyninst_cxx_group_test.dyn_g++_32_none_none ${SOURCE_LIST_50})
add_executable (dyninst_group_test.dyn_g++_32_none_none ${SOURCE_LIST_34})
add_executable (init_fini_callback.dyn_g++_32_none_none ${SOURCE_LIST_57})
add_executable (test1_12.dyn_g++_32_none_none ${SOURCE_LIST_69})
add_executable (test1_14.dyn_g++_32_none_none ${SOURCE_LIST_48})
add_executable (test1_19.dyn_g++_32_none_none ${SOURCE_LIST_24})
add_executable (test1_29.dyn_g++_32_none_none ${SOURCE_LIST_4})
add_executable (test1_35.dyn_g++_32_none_none ${SOURCE_LIST_64})
add_executable (test1_40.dyn_g++_32_none_none ${SOURCE_LIST_39})
add_executable (test1_41.dyn_g++_32_none_none ${SOURCE_LIST_7})
add_executable (test2_14.dyn_g++_32_none_none ${SOURCE_LIST_58})
add_executable (test2_6.dyn_g++_32_none_none ${SOURCE_LIST_1})
add_executable (test2_8.dyn_g++_32_none_none ${SOURCE_LIST_2})
add_executable (test3_1.dyn_g++_32_none_none ${SOURCE_LIST_32})
add_executable (test3_2.dyn_g++_32_none_none ${SOURCE_LIST_38})
add_executable (test3_3.dyn_g++_32_none_none ${SOURCE_LIST_53})
add_executable (test3_4.dyn_g++_32_none_none ${SOURCE_LIST_61})
add_executable (test3_5.dyn_g++_32_none_none ${SOURCE_LIST_6})
add_executable (test3_6.dyn_g++_32_none_none ${SOURCE_LIST_0})
add_executable (test3_7.dyn_g++_32_none_none ${SOURCE_LIST_28})
add_executable (test4_1.dyn_g++_32_none_none ${SOURCE_LIST_3})
add_executable (test4_2.dyn_g++_32_none_none ${SOURCE_LIST_68})
add_executable (test4_3.dyn_g++_32_none_none ${SOURCE_LIST_63})
add_executable (test4_3b.dyn_g++_32_none_none ${SOURCE_LIST_70})
add_executable (test4_4.dyn_g++_32_none_none ${SOURCE_LIST_42})
add_executable (test4_4b.dyn_g++_32_none_none ${SOURCE_LIST_59})
add_executable (test_callback_1.dyn_g++_32_none_none ${SOURCE_LIST_16})
add_executable (test_callback_2.dyn_g++_32_none_none ${SOURCE_LIST_30})
add_executable (test_fork_10.dyn_g++_32_none_none ${SOURCE_LIST_51})
add_executable (test_fork_11.dyn_g++_32_none_none ${SOURCE_LIST_19})
add_executable (test_fork_12.dyn_g++_32_none_none ${SOURCE_LIST_27})
add_executable (test_fork_13.dyn_g++_32_none_none ${SOURCE_LIST_11})
add_executable (test_fork_5.dyn_g++_32_none_none ${SOURCE_LIST_26})
add_executable (test_fork_6.dyn_g++_32_none_none ${SOURCE_LIST_21})
add_executable (test_fork_7.dyn_g++_32_none_none ${SOURCE_LIST_25})
add_executable (test_fork_8.dyn_g++_32_none_none ${SOURCE_LIST_17})
add_executable (test_fork_9.dyn_g++_32_none_none ${SOURCE_LIST_41})
add_executable (test_mem_1.dyn_g++_32_none_none ${SOURCE_LIST_55})
add_executable (test_mem_2.dyn_g++_32_none_none ${SOURCE_LIST_45})
add_executable (test_mem_3.dyn_g++_32_none_none ${SOURCE_LIST_49})
add_executable (test_mem_4.dyn_g++_32_none_none ${SOURCE_LIST_9})
add_executable (test_mem_5.dyn_g++_32_none_none ${SOURCE_LIST_20})
add_executable (test_mem_6.dyn_g++_32_none_none ${SOURCE_LIST_37})
add_executable (test_mem_7.dyn_g++_32_none_none ${SOURCE_LIST_13})
add_executable (test_mem_8.dyn_g++_32_none_none ${SOURCE_LIST_23})
add_executable (test_snip_remove.dyn_g++_32_none_none ${SOURCE_LIST_52})
add_executable (test_stack_1.dyn_g++_32_none_none ${SOURCE_LIST_47})
add_executable (test_stack_2.dyn_g++_32_none_none ${SOURCE_LIST_60})
add_executable (test_stack_3.dyn_g++_32_none_none ${SOURCE_LIST_56})
add_executable (test_stack_4.dyn_g++_32_none_none ${SOURCE_LIST_66})
add_executable (test_thread_1.dyn_g++_32_none_none ${SOURCE_LIST_65})
target_link_libraries (test_thread_1.dyn_g++_32_none_none pthread)
add_executable (test_thread_2.dyn_g++_32_none_none ${SOURCE_LIST_35})
target_link_libraries (test_thread_2.dyn_g++_32_none_none pthread)
add_executable (test_thread_3.dyn_g++_32_none_none ${SOURCE_LIST_14})
target_link_libraries (test_thread_3.dyn_g++_32_none_none pthread)
add_executable (test_thread_5.dyn_g++_32_none_none ${SOURCE_LIST_15})
target_link_libraries (test_thread_5.dyn_g++_32_none_none pthread)
add_executable (test_thread_6.dyn_g++_32_none_none ${SOURCE_LIST_62})
target_link_libraries (test_thread_6.dyn_g++_32_none_none pthread)
add_executable (test_thread_7.dyn_g++_32_none_none ${SOURCE_LIST_67})
target_link_libraries (test_thread_7.dyn_g++_32_none_none pthread)
add_executable (test_thread_8.dyn_g++_32_none_none ${SOURCE_LIST_8})
target_link_libraries (test_thread_8.dyn_g++_32_none_none pthread)


INSTALL (TARGETS
		dyninst_cxx_group_test.dyn_g++_32_none_none
		dyninst_group_test.dyn_g++_32_none_none
		init_fini_callback.dyn_g++_32_none_none
		test1_12.dyn_g++_32_none_none
		test1_14.dyn_g++_32_none_none
		test1_19.dyn_g++_32_none_none
		test1_29.dyn_g++_32_none_none
		test1_35.dyn_g++_32_none_none
		test1_40.dyn_g++_32_none_none
		test1_41.dyn_g++_32_none_none
		test2_14.dyn_g++_32_none_none
		test2_6.dyn_g++_32_none_none
		test2_8.dyn_g++_32_none_none
		test3_1.dyn_g++_32_none_none
		test3_2.dyn_g++_32_none_none
		test3_3.dyn_g++_32_none_none
		test3_4.dyn_g++_32_none_none
		test3_5.dyn_g++_32_none_none
		test3_6.dyn_g++_32_none_none
		test3_7.dyn_g++_32_none_none
		test4_1.dyn_g++_32_none_none
		test4_2.dyn_g++_32_none_none
		test4_3.dyn_g++_32_none_none
		test4_3b.dyn_g++_32_none_none
		test4_4.dyn_g++_32_none_none
		test4_4b.dyn_g++_32_none_none
		test_callback_1.dyn_g++_32_none_none
		test_callback_2.dyn_g++_32_none_none
		test_fork_10.dyn_g++_32_none_none
		test_fork_11.dyn_g++_32_none_none
		test_fork_12.dyn_g++_32_none_none
		test_fork_13.dyn_g++_32_none_none
		test_fork_5.dyn_g++_32_none_none
		test_fork_6.dyn_g++_32_none_none
		test_fork_7.dyn_g++_32_none_none
		test_fork_8.dyn_g++_32_none_none
		test_fork_9.dyn_g++_32_none_none
		test_mem_1.dyn_g++_32_none_none
		test_mem_2.dyn_g++_32_none_none
		test_mem_3.dyn_g++_32_none_none
		test_mem_4.dyn_g++_32_none_none
		test_mem_5.dyn_g++_32_none_none
		test_mem_6.dyn_g++_32_none_none
		test_mem_7.dyn_g++_32_none_none
		test_mem_8.dyn_g++_32_none_none
		test_snip_remove.dyn_g++_32_none_none
		test_stack_1.dyn_g++_32_none_none
		test_stack_2.dyn_g++_32_none_none
		test_stack_3.dyn_g++_32_none_none
		test_stack_4.dyn_g++_32_none_none
		test_thread_1.dyn_g++_32_none_none
		test_thread_2.dyn_g++_32_none_none
		test_thread_3.dyn_g++_32_none_none
		test_thread_5.dyn_g++_32_none_none
		test_thread_6.dyn_g++_32_none_none
		test_thread_7.dyn_g++_32_none_none
		test_thread_8.dyn_g++_32_none_none
	DESTINATION testsuite)
