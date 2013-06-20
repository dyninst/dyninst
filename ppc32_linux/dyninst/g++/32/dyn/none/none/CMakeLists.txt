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
include (${PROJECT_SOURCE_DIR}/ppc32_linux/srclists.cmake)
add_executable (dyninst_cxx_group_test.dyn_g++_32_none_none ${SOURCE_LIST_21})
add_executable (dyninst_group_test.dyn_g++_32_none_none ${SOURCE_LIST_57})
target_link_libraries (dyninst_group_test.dyn_g++_32_none_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_g++_32_none_none ${SOURCE_LIST_52})
target_link_libraries (init_fini_callback.dyn_g++_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_g++_32_none_none ${SOURCE_LIST_64})
add_executable (test1_14.dyn_g++_32_none_none ${SOURCE_LIST_41})
add_executable (test1_19.dyn_g++_32_none_none ${SOURCE_LIST_19})
add_executable (test1_29.dyn_g++_32_none_none ${SOURCE_LIST_4})
add_executable (test1_40.dyn_g++_32_none_none ${SOURCE_LIST_34})
add_executable (test1_41.dyn_g++_32_none_none ${SOURCE_LIST_6})
add_executable (test2_14.dyn_g++_32_none_none ${SOURCE_LIST_53})
add_executable (test2_6.dyn_g++_32_none_none ${SOURCE_LIST_1})
target_link_libraries (test2_6.dyn_g++_32_none_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_g++_32_none_none ${SOURCE_LIST_2})
add_executable (test3_1.dyn_g++_32_none_none ${SOURCE_LIST_28})
add_executable (test3_2.dyn_g++_32_none_none ${SOURCE_LIST_33})
add_executable (test3_3.dyn_g++_32_none_none ${SOURCE_LIST_47})
add_executable (test3_4.dyn_g++_32_none_none ${SOURCE_LIST_56})
add_executable (test3_5.dyn_g++_32_none_none ${SOURCE_LIST_43})
add_executable (test3_6.dyn_g++_32_none_none ${SOURCE_LIST_45})
add_executable (test3_7.dyn_g++_32_none_none ${SOURCE_LIST_24})
add_executable (test4_1.dyn_g++_32_none_none ${SOURCE_LIST_3})
add_executable (test4_2.dyn_g++_32_none_none ${SOURCE_LIST_63})
add_executable (test4_3.dyn_g++_32_none_none ${SOURCE_LIST_59})
add_executable (test4_3b.dyn_g++_32_none_none ${SOURCE_LIST_65})
add_executable (test4_4.dyn_g++_32_none_none ${SOURCE_LIST_36})
add_executable (test4_4b.dyn_g++_32_none_none ${SOURCE_LIST_54})
add_executable (test_callback_1.dyn_g++_32_none_none ${SOURCE_LIST_13})
add_executable (test_callback_2.dyn_g++_32_none_none ${SOURCE_LIST_26})
add_executable (test_fork_10.dyn_g++_32_none_none ${SOURCE_LIST_44})
add_executable (test_fork_11.dyn_g++_32_none_none ${SOURCE_LIST_16})
add_executable (test_fork_12.dyn_g++_32_none_none ${SOURCE_LIST_23})
add_executable (test_fork_13.dyn_g++_32_none_none ${SOURCE_LIST_9})
add_executable (test_fork_5.dyn_g++_32_none_none ${SOURCE_LIST_60})
add_executable (test_fork_6.dyn_g++_32_none_none ${SOURCE_LIST_17})
add_executable (test_fork_7.dyn_g++_32_none_none ${SOURCE_LIST_20})
add_executable (test_fork_8.dyn_g++_32_none_none ${SOURCE_LIST_14})
add_executable (test_fork_9.dyn_g++_32_none_none ${SOURCE_LIST_35})
add_executable (test_snip_remove.dyn_g++_32_none_none ${SOURCE_LIST_46})
add_executable (test_stack_1.dyn_g++_32_none_none ${SOURCE_LIST_40})
add_executable (test_stack_2.dyn_g++_32_none_none ${SOURCE_LIST_55})
add_executable (test_stack_3.dyn_g++_32_none_none ${SOURCE_LIST_49})
add_executable (test_stack_4.dyn_g++_32_none_none ${SOURCE_LIST_61})
add_executable (test_thread_1.dyn_g++_32_none_none ${SOURCE_LIST_30})
target_link_libraries (test_thread_1.dyn_g++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_g++_32_none_none ${SOURCE_LIST_31})
target_link_libraries (test_thread_2.dyn_g++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_g++_32_none_none ${SOURCE_LIST_12})
target_link_libraries (test_thread_3.dyn_g++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_g++_32_none_none ${SOURCE_LIST_11})
target_link_libraries (test_thread_5.dyn_g++_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_g++_32_none_none ${SOURCE_LIST_58})
target_link_libraries (test_thread_6.dyn_g++_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_g++_32_none_none ${SOURCE_LIST_62})
target_link_libraries (test_thread_7.dyn_g++_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_g++_32_none_none ${SOURCE_LIST_7})
target_link_libraries (test_thread_8.dyn_g++_32_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_cxx_group_test.dyn_g++_32_none_none
		dyninst_group_test.dyn_g++_32_none_none
		init_fini_callback.dyn_g++_32_none_none
		test1_12.dyn_g++_32_none_none
		test1_14.dyn_g++_32_none_none
		test1_19.dyn_g++_32_none_none
		test1_29.dyn_g++_32_none_none
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
	DESTINATION ${INSTALL_DIR})
