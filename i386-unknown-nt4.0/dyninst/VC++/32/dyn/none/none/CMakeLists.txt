# CMakeLists for dyninst/VC++/32/dyn/none/none
set (CMAKE_CXX_FLAGS "/D_DEBUG ${MUTATOR_DEFINES}   ${CXXFLAGS}   ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_native_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_native_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_native_cc}"
	"/D_DEBUG ${MUTATOR_DEFINES}   ${CXXFLAGS}   "
	"${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  "
	"CXX"
	"native_cc 32 dyn"
	MUTATEEnativecc32dynCXX)

IF (MUTATEEnativecc32dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
add_executable (dyninst_cxx_group_test.dyn_VC++_32_none_none ${SOURCE_LIST_22})
set_source_files_properties(${SOURCE_LIST_22} PROPERTIES LANGUAGE CXX)
set_target_properties(dyninst_cxx_group_test.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (dyninst_group_test.dyn_VC++_32_none_none ${SOURCE_LIST_41})
set_source_files_properties(${SOURCE_LIST_41} PROPERTIES LANGUAGE CXX)
set_target_properties(dyninst_group_test.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (dyninst_group_test.dyn_VC++_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_VC++_32_none_none ${SOURCE_LIST_42})
set_source_files_properties(${SOURCE_LIST_42} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_12.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_14.dyn_VC++_32_none_none ${SOURCE_LIST_30})
set_source_files_properties(${SOURCE_LIST_30} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_14.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_19.dyn_VC++_32_none_none ${SOURCE_LIST_14})
set_source_files_properties(${SOURCE_LIST_14} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_19.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_29.dyn_VC++_32_none_none ${SOURCE_LIST_4})
set_source_files_properties(${SOURCE_LIST_4} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_29.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test2_14.dyn_VC++_32_none_none ${SOURCE_LIST_35})
set_source_files_properties(${SOURCE_LIST_35} PROPERTIES LANGUAGE CXX)
set_target_properties(test2_14.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test2_8.dyn_VC++_32_none_none ${SOURCE_LIST_2})
set_source_files_properties(${SOURCE_LIST_2} PROPERTIES LANGUAGE CXX)
set_target_properties(test2_8.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test3_1.dyn_VC++_32_none_none ${SOURCE_LIST_20})
set_source_files_properties(${SOURCE_LIST_20} PROPERTIES LANGUAGE CXX)
set_target_properties(test3_1.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test3_2.dyn_VC++_32_none_none ${SOURCE_LIST_24})
set_source_files_properties(${SOURCE_LIST_24} PROPERTIES LANGUAGE CXX)
set_target_properties(test3_2.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test3_3.dyn_VC++_32_none_none ${SOURCE_LIST_32})
set_source_files_properties(${SOURCE_LIST_32} PROPERTIES LANGUAGE CXX)
set_target_properties(test3_3.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test3_4.dyn_VC++_32_none_none ${SOURCE_LIST_37})
set_source_files_properties(${SOURCE_LIST_37} PROPERTIES LANGUAGE CXX)
set_target_properties(test3_4.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test3_7.dyn_VC++_32_none_none ${SOURCE_LIST_16})
set_source_files_properties(${SOURCE_LIST_16} PROPERTIES LANGUAGE CXX)
set_target_properties(test3_7.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test4_1.dyn_VC++_32_none_none ${SOURCE_LIST_3})
set_source_files_properties(${SOURCE_LIST_3} PROPERTIES LANGUAGE CXX)
set_target_properties(test4_1.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_1.dyn_VC++_32_none_none ${SOURCE_LIST_40})
set_source_files_properties(${SOURCE_LIST_40} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_1.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_2.dyn_VC++_32_none_none ${SOURCE_LIST_1})
set_source_files_properties(${SOURCE_LIST_1} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_2.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_3.dyn_VC++_32_none_none ${SOURCE_LIST_15})
set_source_files_properties(${SOURCE_LIST_15} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_3.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_4.dyn_VC++_32_none_none ${SOURCE_LIST_10})
set_source_files_properties(${SOURCE_LIST_10} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_4.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_5.dyn_VC++_32_none_none ${SOURCE_LIST_18})
set_source_files_properties(${SOURCE_LIST_18} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_5.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_6.dyn_VC++_32_none_none ${SOURCE_LIST_39})
set_source_files_properties(${SOURCE_LIST_39} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_6.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_7.dyn_VC++_32_none_none ${SOURCE_LIST_7})
set_source_files_properties(${SOURCE_LIST_7} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_7.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_mem_8.dyn_VC++_32_none_none ${SOURCE_LIST_8})
set_source_files_properties(${SOURCE_LIST_8} PROPERTIES LANGUAGE CXX)
set_target_properties(test_mem_8.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_snip_remove.dyn_VC++_32_none_none ${SOURCE_LIST_31})
set_source_files_properties(${SOURCE_LIST_31} PROPERTIES LANGUAGE CXX)
set_target_properties(test_snip_remove.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_stack_1.dyn_VC++_32_none_none ${SOURCE_LIST_36})
set_source_files_properties(${SOURCE_LIST_36} PROPERTIES LANGUAGE CXX)
set_target_properties(test_stack_1.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_stack_3.dyn_VC++_32_none_none ${SOURCE_LIST_34})
set_source_files_properties(${SOURCE_LIST_34} PROPERTIES LANGUAGE CXX)
set_target_properties(test_stack_3.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_thread_6.dyn_VC++_32_none_none ${SOURCE_LIST_38})
set_source_files_properties(${SOURCE_LIST_38} PROPERTIES LANGUAGE CXX)
set_target_properties(test_thread_6.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_thread_7.dyn_VC++_32_none_none ${SOURCE_LIST_26})
set_source_files_properties(${SOURCE_LIST_26} PROPERTIES LANGUAGE CXX)
set_target_properties(test_thread_7.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test_thread_8.dyn_VC++_32_none_none ${SOURCE_LIST_6})
set_source_files_properties(${SOURCE_LIST_6} PROPERTIES LANGUAGE CXX)
set_target_properties(test_thread_8.dyn_VC++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		dyninst_cxx_group_test.dyn_VC++_32_none_none
		dyninst_group_test.dyn_VC++_32_none_none
		test1_12.dyn_VC++_32_none_none
		test1_14.dyn_VC++_32_none_none
		test1_19.dyn_VC++_32_none_none
		test1_29.dyn_VC++_32_none_none
		test2_14.dyn_VC++_32_none_none
		test2_8.dyn_VC++_32_none_none
		test3_1.dyn_VC++_32_none_none
		test3_2.dyn_VC++_32_none_none
		test3_3.dyn_VC++_32_none_none
		test3_4.dyn_VC++_32_none_none
		test3_7.dyn_VC++_32_none_none
		test4_1.dyn_VC++_32_none_none
		test_mem_1.dyn_VC++_32_none_none
		test_mem_2.dyn_VC++_32_none_none
		test_mem_3.dyn_VC++_32_none_none
		test_mem_4.dyn_VC++_32_none_none
		test_mem_5.dyn_VC++_32_none_none
		test_mem_6.dyn_VC++_32_none_none
		test_mem_7.dyn_VC++_32_none_none
		test_mem_8.dyn_VC++_32_none_none
		test_snip_remove.dyn_VC++_32_none_none
		test_stack_1.dyn_VC++_32_none_none
		test_stack_3.dyn_VC++_32_none_none
		test_thread_6.dyn_VC++_32_none_none
		test_thread_7.dyn_VC++_32_none_none
		test_thread_8.dyn_VC++_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for native_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler native_cc not present")
ENDIF()
