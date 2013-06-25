# CMakeLists for dyninst/VC/32/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  -TC ${CFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_native_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_C_LINK_EXECUTABLE ${M_native_linker})
set (CMAKE_CXX_LINK_EXECUTABLE ${M_native_linker})
set (CMAKE_EXE_LINKER_FLAGS "${LDFLAGS} ${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
add_executable (dyninst_group_test.dyn_VC_32_none_none.exe ${SOURCE_LIST_41})
target_link_libraries (dyninst_group_test.dyn_VC_32_none_none.exe ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_VC_32_none_none.exe ${SOURCE_LIST_42})
add_executable (test1_14.dyn_VC_32_none_none.exe ${SOURCE_LIST_30})
add_executable (test1_19.dyn_VC_32_none_none.exe ${SOURCE_LIST_14})
add_executable (test1_29.dyn_VC_32_none_none.exe ${SOURCE_LIST_4})
add_executable (test2_14.dyn_VC_32_none_none.exe ${SOURCE_LIST_35})
add_executable (test2_8.dyn_VC_32_none_none.exe ${SOURCE_LIST_2})
add_executable (test3_1.dyn_VC_32_none_none.exe ${SOURCE_LIST_20})
add_executable (test3_2.dyn_VC_32_none_none.exe ${SOURCE_LIST_24})
add_executable (test3_3.dyn_VC_32_none_none.exe ${SOURCE_LIST_32})
add_executable (test3_4.dyn_VC_32_none_none.exe ${SOURCE_LIST_37})
add_executable (test3_7.dyn_VC_32_none_none.exe ${SOURCE_LIST_16})
add_executable (test4_1.dyn_VC_32_none_none.exe ${SOURCE_LIST_3})
add_executable (test_mem_1.dyn_VC_32_none_none.exe ${SOURCE_LIST_40})
add_executable (test_mem_2.dyn_VC_32_none_none.exe ${SOURCE_LIST_1})
add_executable (test_mem_3.dyn_VC_32_none_none.exe ${SOURCE_LIST_15})
add_executable (test_mem_4.dyn_VC_32_none_none.exe ${SOURCE_LIST_10})
add_executable (test_mem_5.dyn_VC_32_none_none.exe ${SOURCE_LIST_18})
add_executable (test_mem_6.dyn_VC_32_none_none.exe ${SOURCE_LIST_39})
add_executable (test_mem_7.dyn_VC_32_none_none.exe ${SOURCE_LIST_7})
add_executable (test_mem_8.dyn_VC_32_none_none.exe ${SOURCE_LIST_8})
add_executable (test_snip_remove.dyn_VC_32_none_none.exe ${SOURCE_LIST_31})
add_executable (test_stack_1.dyn_VC_32_none_none.exe ${SOURCE_LIST_36})
add_executable (test_stack_3.dyn_VC_32_none_none.exe ${SOURCE_LIST_34})
add_executable (test_thread_6.dyn_VC_32_none_none.exe ${SOURCE_LIST_38})
add_executable (test_thread_7.dyn_VC_32_none_none.exe ${SOURCE_LIST_26})
add_executable (test_thread_8.dyn_VC_32_none_none.exe ${SOURCE_LIST_6})


INSTALL (TARGETS
		dyninst_group_test.dyn_VC_32_none_none.exe
		test1_12.dyn_VC_32_none_none.exe
		test1_14.dyn_VC_32_none_none.exe
		test1_19.dyn_VC_32_none_none.exe
		test1_29.dyn_VC_32_none_none.exe
		test2_14.dyn_VC_32_none_none.exe
		test2_8.dyn_VC_32_none_none.exe
		test3_1.dyn_VC_32_none_none.exe
		test3_2.dyn_VC_32_none_none.exe
		test3_3.dyn_VC_32_none_none.exe
		test3_4.dyn_VC_32_none_none.exe
		test3_7.dyn_VC_32_none_none.exe
		test4_1.dyn_VC_32_none_none.exe
		test_mem_1.dyn_VC_32_none_none.exe
		test_mem_2.dyn_VC_32_none_none.exe
		test_mem_3.dyn_VC_32_none_none.exe
		test_mem_4.dyn_VC_32_none_none.exe
		test_mem_5.dyn_VC_32_none_none.exe
		test_mem_6.dyn_VC_32_none_none.exe
		test_mem_7.dyn_VC_32_none_none.exe
		test_mem_8.dyn_VC_32_none_none.exe
		test_snip_remove.dyn_VC_32_none_none.exe
		test_stack_1.dyn_VC_32_none_none.exe
		test_stack_3.dyn_VC_32_none_none.exe
		test_thread_6.dyn_VC_32_none_none.exe
		test_thread_7.dyn_VC_32_none_none.exe
		test_thread_8.dyn_VC_32_none_none.exe
	DESTINATION ${INSTALL_DIR})
