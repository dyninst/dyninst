# CMakeLists for dyninst/pgcc/64/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_pg_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS_NATIVE} ${MUTATEE_CFLAGS_NATIVE}    "
	"${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"C"
	"pg_cc 64 dyn"
	MUTATEEpgcc64dynC)

IF (MUTATEEpgcc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (amd64_7_arg_call.dyn_pgcc_64_none_none ${SOURCE_LIST_47})
set_source_files_properties(${SOURCE_LIST_47} PROPERTIES LANGUAGE C)
set_target_properties(amd64_7_arg_call.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (dyninst_group_test.dyn_pgcc_64_none_none ${SOURCE_LIST_48})
set_source_files_properties(${SOURCE_LIST_48} PROPERTIES LANGUAGE C)
set_target_properties(dyninst_group_test.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (dyninst_group_test.dyn_pgcc_64_none_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_pgcc_64_none_none ${SOURCE_LIST_64})
set_source_files_properties(${SOURCE_LIST_64} PROPERTIES LANGUAGE C)
set_target_properties(init_fini_callback.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (init_fini_callback.dyn_pgcc_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_pgcc_64_none_none ${SOURCE_LIST_77})
set_source_files_properties(${SOURCE_LIST_77} PROPERTIES LANGUAGE C)
set_target_properties(test1_12.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_14.dyn_pgcc_64_none_none ${SOURCE_LIST_53})
set_source_files_properties(${SOURCE_LIST_53} PROPERTIES LANGUAGE C)
set_target_properties(test1_14.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_19.dyn_pgcc_64_none_none ${SOURCE_LIST_26})
set_source_files_properties(${SOURCE_LIST_26} PROPERTIES LANGUAGE C)
set_target_properties(test1_19.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_29.dyn_pgcc_64_none_none ${SOURCE_LIST_4})
set_source_files_properties(${SOURCE_LIST_4} PROPERTIES LANGUAGE C)
set_target_properties(test1_29.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_35.dyn_pgcc_64_none_none ${SOURCE_LIST_44})
set_source_files_properties(${SOURCE_LIST_44} PROPERTIES LANGUAGE C)
set_target_properties(test1_35.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_40.dyn_pgcc_64_none_none ${SOURCE_LIST_41})
set_source_files_properties(${SOURCE_LIST_41} PROPERTIES LANGUAGE C)
set_target_properties(test1_40.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_41.dyn_pgcc_64_none_none ${SOURCE_LIST_8})
set_source_files_properties(${SOURCE_LIST_8} PROPERTIES LANGUAGE C)
set_target_properties(test1_41.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_14.dyn_pgcc_64_none_none ${SOURCE_LIST_65})
set_source_files_properties(${SOURCE_LIST_65} PROPERTIES LANGUAGE C)
set_target_properties(test2_14.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_6.dyn_pgcc_64_none_none ${SOURCE_LIST_1})
set_source_files_properties(${SOURCE_LIST_1} PROPERTIES LANGUAGE C)
set_target_properties(test2_6.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test2_6.dyn_pgcc_64_none_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_pgcc_64_none_none ${SOURCE_LIST_2})
set_source_files_properties(${SOURCE_LIST_2} PROPERTIES LANGUAGE C)
set_target_properties(test2_8.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_1.dyn_pgcc_64_none_none ${SOURCE_LIST_25})
set_source_files_properties(${SOURCE_LIST_25} PROPERTIES LANGUAGE C)
set_target_properties(test3_1.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_2.dyn_pgcc_64_none_none ${SOURCE_LIST_40})
set_source_files_properties(${SOURCE_LIST_40} PROPERTIES LANGUAGE C)
set_target_properties(test3_2.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_3.dyn_pgcc_64_none_none ${SOURCE_LIST_60})
set_source_files_properties(${SOURCE_LIST_60} PROPERTIES LANGUAGE C)
set_target_properties(test3_3.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_4.dyn_pgcc_64_none_none ${SOURCE_LIST_68})
set_source_files_properties(${SOURCE_LIST_68} PROPERTIES LANGUAGE C)
set_target_properties(test3_4.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_5.dyn_pgcc_64_none_none ${SOURCE_LIST_55})
set_source_files_properties(${SOURCE_LIST_55} PROPERTIES LANGUAGE C)
set_target_properties(test3_5.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_6.dyn_pgcc_64_none_none ${SOURCE_LIST_58})
set_source_files_properties(${SOURCE_LIST_58} PROPERTIES LANGUAGE C)
set_target_properties(test3_6.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_7.dyn_pgcc_64_none_none ${SOURCE_LIST_31})
set_source_files_properties(${SOURCE_LIST_31} PROPERTIES LANGUAGE C)
set_target_properties(test3_7.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_1.dyn_pgcc_64_none_none ${SOURCE_LIST_3})
set_source_files_properties(${SOURCE_LIST_3} PROPERTIES LANGUAGE C)
set_target_properties(test4_1.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_2.dyn_pgcc_64_none_none ${SOURCE_LIST_76})
set_source_files_properties(${SOURCE_LIST_76} PROPERTIES LANGUAGE C)
set_target_properties(test4_2.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3.dyn_pgcc_64_none_none ${SOURCE_LIST_71})
set_source_files_properties(${SOURCE_LIST_71} PROPERTIES LANGUAGE C)
set_target_properties(test4_3.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3b.dyn_pgcc_64_none_none ${SOURCE_LIST_78})
set_source_files_properties(${SOURCE_LIST_78} PROPERTIES LANGUAGE C)
set_target_properties(test4_3b.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4.dyn_pgcc_64_none_none ${SOURCE_LIST_45})
set_source_files_properties(${SOURCE_LIST_45} PROPERTIES LANGUAGE C)
set_target_properties(test4_4.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4b.dyn_pgcc_64_none_none ${SOURCE_LIST_66})
set_source_files_properties(${SOURCE_LIST_66} PROPERTIES LANGUAGE C)
set_target_properties(test4_4b.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_1.dyn_pgcc_64_none_none ${SOURCE_LIST_18})
set_source_files_properties(${SOURCE_LIST_18} PROPERTIES LANGUAGE C)
set_target_properties(test_callback_1.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_2.dyn_pgcc_64_none_none ${SOURCE_LIST_33})
set_source_files_properties(${SOURCE_LIST_33} PROPERTIES LANGUAGE C)
set_target_properties(test_callback_2.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_10.dyn_pgcc_64_none_none ${SOURCE_LIST_56})
set_source_files_properties(${SOURCE_LIST_56} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_10.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_11.dyn_pgcc_64_none_none ${SOURCE_LIST_21})
set_source_files_properties(${SOURCE_LIST_21} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_11.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_12.dyn_pgcc_64_none_none ${SOURCE_LIST_30})
set_source_files_properties(${SOURCE_LIST_30} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_12.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_13.dyn_pgcc_64_none_none ${SOURCE_LIST_14})
set_source_files_properties(${SOURCE_LIST_14} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_13.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_14.dyn_pgcc_64_none_none ${SOURCE_LIST_23})
set_source_files_properties(${SOURCE_LIST_23} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_14.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_5.dyn_pgcc_64_none_none ${SOURCE_LIST_72})
set_source_files_properties(${SOURCE_LIST_72} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_5.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_6.dyn_pgcc_64_none_none ${SOURCE_LIST_57})
set_source_files_properties(${SOURCE_LIST_57} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_6.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_7.dyn_pgcc_64_none_none ${SOURCE_LIST_27})
set_source_files_properties(${SOURCE_LIST_27} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_7.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_8.dyn_pgcc_64_none_none ${SOURCE_LIST_19})
set_source_files_properties(${SOURCE_LIST_19} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_8.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_9.dyn_pgcc_64_none_none ${SOURCE_LIST_43})
set_source_files_properties(${SOURCE_LIST_43} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_9.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_1.dyn_pgcc_64_none_none ${SOURCE_LIST_42})
set_source_files_properties(${SOURCE_LIST_42} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_1.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_2.dyn_pgcc_64_none_none ${SOURCE_LIST_7})
set_source_files_properties(${SOURCE_LIST_7} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_2.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_3.dyn_pgcc_64_none_none ${SOURCE_LIST_10})
set_source_files_properties(${SOURCE_LIST_10} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_3.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_4.dyn_pgcc_64_none_none ${SOURCE_LIST_75})
set_source_files_properties(${SOURCE_LIST_75} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_4.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_5.dyn_pgcc_64_none_none ${SOURCE_LIST_62})
set_source_files_properties(${SOURCE_LIST_62} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_5.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_6.dyn_pgcc_64_none_none ${SOURCE_LIST_29})
set_source_files_properties(${SOURCE_LIST_29} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_6.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_7.dyn_pgcc_64_none_none ${SOURCE_LIST_22})
set_source_files_properties(${SOURCE_LIST_22} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_7.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_8.dyn_pgcc_64_none_none ${SOURCE_LIST_38})
set_source_files_properties(${SOURCE_LIST_38} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_8.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_snip_remove.dyn_pgcc_64_none_none ${SOURCE_LIST_59})
set_source_files_properties(${SOURCE_LIST_59} PROPERTIES LANGUAGE C)
set_target_properties(test_snip_remove.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_1.dyn_pgcc_64_none_none ${SOURCE_LIST_52})
set_source_files_properties(${SOURCE_LIST_52} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_1.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_2.dyn_pgcc_64_none_none ${SOURCE_LIST_67})
set_source_files_properties(${SOURCE_LIST_67} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_2.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_3.dyn_pgcc_64_none_none ${SOURCE_LIST_5})
set_source_files_properties(${SOURCE_LIST_5} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_3.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_4.dyn_pgcc_64_none_none ${SOURCE_LIST_73})
set_source_files_properties(${SOURCE_LIST_73} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_4.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_thread_1.dyn_pgcc_64_none_none ${SOURCE_LIST_37})
set_source_files_properties(${SOURCE_LIST_37} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_1.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_1.dyn_pgcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_pgcc_64_none_none ${SOURCE_LIST_12})
set_source_files_properties(${SOURCE_LIST_12} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_2.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_2.dyn_pgcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_pgcc_64_none_none ${SOURCE_LIST_17})
set_source_files_properties(${SOURCE_LIST_17} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_3.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_3.dyn_pgcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_pgcc_64_none_none ${SOURCE_LIST_16})
set_source_files_properties(${SOURCE_LIST_16} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_5.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_5.dyn_pgcc_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_pgcc_64_none_none ${SOURCE_LIST_69})
set_source_files_properties(${SOURCE_LIST_69} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_6.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_6.dyn_pgcc_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_pgcc_64_none_none ${SOURCE_LIST_74})
set_source_files_properties(${SOURCE_LIST_74} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_7.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_7.dyn_pgcc_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_pgcc_64_none_none ${SOURCE_LIST_11})
set_source_files_properties(${SOURCE_LIST_11} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_8.dyn_pgcc_64_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_8.dyn_pgcc_64_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		amd64_7_arg_call.dyn_pgcc_64_none_none
		dyninst_group_test.dyn_pgcc_64_none_none
		init_fini_callback.dyn_pgcc_64_none_none
		test1_12.dyn_pgcc_64_none_none
		test1_14.dyn_pgcc_64_none_none
		test1_19.dyn_pgcc_64_none_none
		test1_29.dyn_pgcc_64_none_none
		test1_35.dyn_pgcc_64_none_none
		test1_40.dyn_pgcc_64_none_none
		test1_41.dyn_pgcc_64_none_none
		test2_14.dyn_pgcc_64_none_none
		test2_6.dyn_pgcc_64_none_none
		test2_8.dyn_pgcc_64_none_none
		test3_1.dyn_pgcc_64_none_none
		test3_2.dyn_pgcc_64_none_none
		test3_3.dyn_pgcc_64_none_none
		test3_4.dyn_pgcc_64_none_none
		test3_5.dyn_pgcc_64_none_none
		test3_6.dyn_pgcc_64_none_none
		test3_7.dyn_pgcc_64_none_none
		test4_1.dyn_pgcc_64_none_none
		test4_2.dyn_pgcc_64_none_none
		test4_3.dyn_pgcc_64_none_none
		test4_3b.dyn_pgcc_64_none_none
		test4_4.dyn_pgcc_64_none_none
		test4_4b.dyn_pgcc_64_none_none
		test_callback_1.dyn_pgcc_64_none_none
		test_callback_2.dyn_pgcc_64_none_none
		test_fork_10.dyn_pgcc_64_none_none
		test_fork_11.dyn_pgcc_64_none_none
		test_fork_12.dyn_pgcc_64_none_none
		test_fork_13.dyn_pgcc_64_none_none
		test_fork_14.dyn_pgcc_64_none_none
		test_fork_5.dyn_pgcc_64_none_none
		test_fork_6.dyn_pgcc_64_none_none
		test_fork_7.dyn_pgcc_64_none_none
		test_fork_8.dyn_pgcc_64_none_none
		test_fork_9.dyn_pgcc_64_none_none
		test_mem_1.dyn_pgcc_64_none_none
		test_mem_2.dyn_pgcc_64_none_none
		test_mem_3.dyn_pgcc_64_none_none
		test_mem_4.dyn_pgcc_64_none_none
		test_mem_5.dyn_pgcc_64_none_none
		test_mem_6.dyn_pgcc_64_none_none
		test_mem_7.dyn_pgcc_64_none_none
		test_mem_8.dyn_pgcc_64_none_none
		test_snip_remove.dyn_pgcc_64_none_none
		test_stack_1.dyn_pgcc_64_none_none
		test_stack_2.dyn_pgcc_64_none_none
		test_stack_3.dyn_pgcc_64_none_none
		test_stack_4.dyn_pgcc_64_none_none
		test_thread_1.dyn_pgcc_64_none_none
		test_thread_2.dyn_pgcc_64_none_none
		test_thread_3.dyn_pgcc_64_none_none
		test_thread_5.dyn_pgcc_64_none_none
		test_thread_6.dyn_pgcc_64_none_none
		test_thread_7.dyn_pgcc_64_none_none
		test_thread_8.dyn_pgcc_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for pg_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler pg_cc not present")
ENDIF()
