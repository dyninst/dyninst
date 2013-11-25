# CMakeLists for dyninst/icc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic")
IF (NOT ${M_intel_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_cc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CFLAGS} -DSOLO_MUTATEE ${MUTATEE_CFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Xlinker -export-dynamic"
	MUTATEEintelcc32dyn)

IF (MUTATEEintelcc32dyn)
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_group_test.dyn_icc_32_pic_none ${SOURCE_LIST_69})
target_link_libraries (dyninst_group_test.dyn_icc_32_pic_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_icc_32_pic_none ${SOURCE_LIST_74})
target_link_libraries (init_fini_callback.dyn_icc_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_icc_32_pic_none ${SOURCE_LIST_78})
add_executable (test1_14.dyn_icc_32_pic_none ${SOURCE_LIST_70})
add_executable (test1_19.dyn_icc_32_pic_none ${SOURCE_LIST_39})
add_executable (test1_29.dyn_icc_32_pic_none ${SOURCE_LIST_58})
add_executable (test1_35.dyn_icc_32_pic_none ${SOURCE_LIST_91})
add_executable (test1_40.dyn_icc_32_pic_none ${SOURCE_LIST_47})
add_executable (test1_41.dyn_icc_32_pic_none ${SOURCE_LIST_83})
add_executable (test2_14.dyn_icc_32_pic_none ${SOURCE_LIST_75})
add_executable (test2_6.dyn_icc_32_pic_none ${SOURCE_LIST_0})
target_link_libraries (test2_6.dyn_icc_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_icc_32_pic_none ${SOURCE_LIST_64})
add_executable (test3_1.dyn_icc_32_pic_none ${SOURCE_LIST_43})
add_executable (test3_2.dyn_icc_32_pic_none ${SOURCE_LIST_46})
add_executable (test3_3.dyn_icc_32_pic_none ${SOURCE_LIST_95})
add_executable (test3_4.dyn_icc_32_pic_none ${SOURCE_LIST_54})
add_executable (test3_5.dyn_icc_32_pic_none ${SOURCE_LIST_94})
add_executable (test3_6.dyn_icc_32_pic_none ${SOURCE_LIST_33})
add_executable (test3_7.dyn_icc_32_pic_none ${SOURCE_LIST_82})
add_executable (test4_1.dyn_icc_32_pic_none ${SOURCE_LIST_1})
add_executable (test4_2.dyn_icc_32_pic_none ${SOURCE_LIST_56})
add_executable (test4_3.dyn_icc_32_pic_none ${SOURCE_LIST_101})
add_executable (test4_3b.dyn_icc_32_pic_none ${SOURCE_LIST_31})
add_executable (test4_4.dyn_icc_32_pic_none ${SOURCE_LIST_50})
add_executable (test4_4b.dyn_icc_32_pic_none ${SOURCE_LIST_67})
add_executable (test_callback_1.dyn_icc_32_pic_none ${SOURCE_LIST_62})
add_executable (test_callback_2.dyn_icc_32_pic_none ${SOURCE_LIST_66})
add_executable (test_fork_10.dyn_icc_32_pic_none ${SOURCE_LIST_71})
add_executable (test_fork_11.dyn_icc_32_pic_none ${SOURCE_LIST_88})
add_executable (test_fork_12.dyn_icc_32_pic_none ${SOURCE_LIST_6})
add_executable (test_fork_13.dyn_icc_32_pic_none ${SOURCE_LIST_59})
add_executable (test_fork_5.dyn_icc_32_pic_none ${SOURCE_LIST_28})
add_executable (test_fork_6.dyn_icc_32_pic_none ${SOURCE_LIST_7})
add_executable (test_fork_7.dyn_icc_32_pic_none ${SOURCE_LIST_40})
add_executable (test_fork_8.dyn_icc_32_pic_none ${SOURCE_LIST_24})
add_executable (test_fork_9.dyn_icc_32_pic_none ${SOURCE_LIST_48})
add_executable (test_mem_1.dyn_icc_32_pic_none ${SOURCE_LIST_19})
add_executable (test_mem_2.dyn_icc_32_pic_none ${SOURCE_LIST_18})
add_executable (test_mem_3.dyn_icc_32_pic_none ${SOURCE_LIST_22})
add_executable (test_mem_4.dyn_icc_32_pic_none ${SOURCE_LIST_32})
add_executable (test_mem_5.dyn_icc_32_pic_none ${SOURCE_LIST_34})
add_executable (test_mem_6.dyn_icc_32_pic_none ${SOURCE_LIST_16})
add_executable (test_mem_7.dyn_icc_32_pic_none ${SOURCE_LIST_37})
add_executable (test_mem_8.dyn_icc_32_pic_none ${SOURCE_LIST_9})
add_executable (test_snip_remove.dyn_icc_32_pic_none ${SOURCE_LIST_72})
add_executable (test_stack_1.dyn_icc_32_pic_none ${SOURCE_LIST_25})
add_executable (test_stack_2.dyn_icc_32_pic_none ${SOURCE_LIST_26})
add_executable (test_stack_3.dyn_icc_32_pic_none ${SOURCE_LIST_97})
add_executable (test_stack_4.dyn_icc_32_pic_none ${SOURCE_LIST_85})
add_executable (test_thread_1.dyn_icc_32_pic_none ${SOURCE_LIST_15})
target_link_libraries (test_thread_1.dyn_icc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_icc_32_pic_none ${SOURCE_LIST_90})
target_link_libraries (test_thread_2.dyn_icc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_icc_32_pic_none ${SOURCE_LIST_61})
target_link_libraries (test_thread_3.dyn_icc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_icc_32_pic_none ${SOURCE_LIST_86})
target_link_libraries (test_thread_5.dyn_icc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_icc_32_pic_none ${SOURCE_LIST_36})
target_link_libraries (test_thread_6.dyn_icc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_icc_32_pic_none ${SOURCE_LIST_29})
target_link_libraries (test_thread_7.dyn_icc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_icc_32_pic_none ${SOURCE_LIST_5})
target_link_libraries (test_thread_8.dyn_icc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_group_test.dyn_icc_32_pic_none
		init_fini_callback.dyn_icc_32_pic_none
		test1_12.dyn_icc_32_pic_none
		test1_14.dyn_icc_32_pic_none
		test1_19.dyn_icc_32_pic_none
		test1_29.dyn_icc_32_pic_none
		test1_35.dyn_icc_32_pic_none
		test1_40.dyn_icc_32_pic_none
		test1_41.dyn_icc_32_pic_none
		test2_14.dyn_icc_32_pic_none
		test2_6.dyn_icc_32_pic_none
		test2_8.dyn_icc_32_pic_none
		test3_1.dyn_icc_32_pic_none
		test3_2.dyn_icc_32_pic_none
		test3_3.dyn_icc_32_pic_none
		test3_4.dyn_icc_32_pic_none
		test3_5.dyn_icc_32_pic_none
		test3_6.dyn_icc_32_pic_none
		test3_7.dyn_icc_32_pic_none
		test4_1.dyn_icc_32_pic_none
		test4_2.dyn_icc_32_pic_none
		test4_3.dyn_icc_32_pic_none
		test4_3b.dyn_icc_32_pic_none
		test4_4.dyn_icc_32_pic_none
		test4_4b.dyn_icc_32_pic_none
		test_callback_1.dyn_icc_32_pic_none
		test_callback_2.dyn_icc_32_pic_none
		test_fork_10.dyn_icc_32_pic_none
		test_fork_11.dyn_icc_32_pic_none
		test_fork_12.dyn_icc_32_pic_none
		test_fork_13.dyn_icc_32_pic_none
		test_fork_5.dyn_icc_32_pic_none
		test_fork_6.dyn_icc_32_pic_none
		test_fork_7.dyn_icc_32_pic_none
		test_fork_8.dyn_icc_32_pic_none
		test_fork_9.dyn_icc_32_pic_none
		test_mem_1.dyn_icc_32_pic_none
		test_mem_2.dyn_icc_32_pic_none
		test_mem_3.dyn_icc_32_pic_none
		test_mem_4.dyn_icc_32_pic_none
		test_mem_5.dyn_icc_32_pic_none
		test_mem_6.dyn_icc_32_pic_none
		test_mem_7.dyn_icc_32_pic_none
		test_mem_8.dyn_icc_32_pic_none
		test_snip_remove.dyn_icc_32_pic_none
		test_stack_1.dyn_icc_32_pic_none
		test_stack_2.dyn_icc_32_pic_none
		test_stack_3.dyn_icc_32_pic_none
		test_stack_4.dyn_icc_32_pic_none
		test_thread_1.dyn_icc_32_pic_none
		test_thread_2.dyn_icc_32_pic_none
		test_thread_3.dyn_icc_32_pic_none
		test_thread_5.dyn_icc_32_pic_none
		test_thread_6.dyn_icc_32_pic_none
		test_thread_7.dyn_icc_32_pic_none
		test_thread_8.dyn_icc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
