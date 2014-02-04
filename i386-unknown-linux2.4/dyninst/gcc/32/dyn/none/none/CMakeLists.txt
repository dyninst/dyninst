# CMakeLists for dyninst/gcc/32/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"C"
	"gnu_cc 32 dyn"
	MUTATEEgnucc32dynC)

IF (MUTATEEgnucc32dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_group_test.dyn_gcc_32_none_none ${SOURCE_LIST_69})
set_source_files_properties(${SOURCE_LIST_69} PROPERTIES LANGUAGE C)
set_target_properties(dyninst_group_test.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (dyninst_group_test.dyn_gcc_32_none_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_gcc_32_none_none ${SOURCE_LIST_74})
set_source_files_properties(${SOURCE_LIST_74} PROPERTIES LANGUAGE C)
set_target_properties(init_fini_callback.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (init_fini_callback.dyn_gcc_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_gcc_32_none_none ${SOURCE_LIST_78})
set_source_files_properties(${SOURCE_LIST_78} PROPERTIES LANGUAGE C)
set_target_properties(test1_12.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_14.dyn_gcc_32_none_none ${SOURCE_LIST_70})
set_source_files_properties(${SOURCE_LIST_70} PROPERTIES LANGUAGE C)
set_target_properties(test1_14.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_19.dyn_gcc_32_none_none ${SOURCE_LIST_39})
set_source_files_properties(${SOURCE_LIST_39} PROPERTIES LANGUAGE C)
set_target_properties(test1_19.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_29.dyn_gcc_32_none_none ${SOURCE_LIST_58})
set_source_files_properties(${SOURCE_LIST_58} PROPERTIES LANGUAGE C)
set_target_properties(test1_29.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_35.dyn_gcc_32_none_none ${SOURCE_LIST_91})
set_source_files_properties(${SOURCE_LIST_91} PROPERTIES LANGUAGE C)
set_target_properties(test1_35.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_40.dyn_gcc_32_none_none ${SOURCE_LIST_47})
set_source_files_properties(${SOURCE_LIST_47} PROPERTIES LANGUAGE C)
set_target_properties(test1_40.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_41.dyn_gcc_32_none_none ${SOURCE_LIST_83})
set_source_files_properties(${SOURCE_LIST_83} PROPERTIES LANGUAGE C)
set_target_properties(test1_41.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_14.dyn_gcc_32_none_none ${SOURCE_LIST_75})
set_source_files_properties(${SOURCE_LIST_75} PROPERTIES LANGUAGE C)
set_target_properties(test2_14.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_6.dyn_gcc_32_none_none ${SOURCE_LIST_0})
set_source_files_properties(${SOURCE_LIST_0} PROPERTIES LANGUAGE C)
set_target_properties(test2_6.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test2_6.dyn_gcc_32_none_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_gcc_32_none_none ${SOURCE_LIST_64})
set_source_files_properties(${SOURCE_LIST_64} PROPERTIES LANGUAGE C)
set_target_properties(test2_8.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_1.dyn_gcc_32_none_none ${SOURCE_LIST_43})
set_source_files_properties(${SOURCE_LIST_43} PROPERTIES LANGUAGE C)
set_target_properties(test3_1.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_2.dyn_gcc_32_none_none ${SOURCE_LIST_46})
set_source_files_properties(${SOURCE_LIST_46} PROPERTIES LANGUAGE C)
set_target_properties(test3_2.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_3.dyn_gcc_32_none_none ${SOURCE_LIST_96})
set_source_files_properties(${SOURCE_LIST_96} PROPERTIES LANGUAGE C)
set_target_properties(test3_3.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_4.dyn_gcc_32_none_none ${SOURCE_LIST_54})
set_source_files_properties(${SOURCE_LIST_54} PROPERTIES LANGUAGE C)
set_target_properties(test3_4.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_5.dyn_gcc_32_none_none ${SOURCE_LIST_94})
set_source_files_properties(${SOURCE_LIST_94} PROPERTIES LANGUAGE C)
set_target_properties(test3_5.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_6.dyn_gcc_32_none_none ${SOURCE_LIST_33})
set_source_files_properties(${SOURCE_LIST_33} PROPERTIES LANGUAGE C)
set_target_properties(test3_6.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_7.dyn_gcc_32_none_none ${SOURCE_LIST_82})
set_source_files_properties(${SOURCE_LIST_82} PROPERTIES LANGUAGE C)
set_target_properties(test3_7.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_1.dyn_gcc_32_none_none ${SOURCE_LIST_1})
set_source_files_properties(${SOURCE_LIST_1} PROPERTIES LANGUAGE C)
set_target_properties(test4_1.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_2.dyn_gcc_32_none_none ${SOURCE_LIST_56})
set_source_files_properties(${SOURCE_LIST_56} PROPERTIES LANGUAGE C)
set_target_properties(test4_2.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3.dyn_gcc_32_none_none ${SOURCE_LIST_102})
set_source_files_properties(${SOURCE_LIST_102} PROPERTIES LANGUAGE C)
set_target_properties(test4_3.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3b.dyn_gcc_32_none_none ${SOURCE_LIST_31})
set_source_files_properties(${SOURCE_LIST_31} PROPERTIES LANGUAGE C)
set_target_properties(test4_3b.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4.dyn_gcc_32_none_none ${SOURCE_LIST_50})
set_source_files_properties(${SOURCE_LIST_50} PROPERTIES LANGUAGE C)
set_target_properties(test4_4.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4b.dyn_gcc_32_none_none ${SOURCE_LIST_67})
set_source_files_properties(${SOURCE_LIST_67} PROPERTIES LANGUAGE C)
set_target_properties(test4_4b.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_1.dyn_gcc_32_none_none ${SOURCE_LIST_62})
set_source_files_properties(${SOURCE_LIST_62} PROPERTIES LANGUAGE C)
set_target_properties(test_callback_1.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_2.dyn_gcc_32_none_none ${SOURCE_LIST_66})
set_source_files_properties(${SOURCE_LIST_66} PROPERTIES LANGUAGE C)
set_target_properties(test_callback_2.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_10.dyn_gcc_32_none_none ${SOURCE_LIST_71})
set_source_files_properties(${SOURCE_LIST_71} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_10.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_11.dyn_gcc_32_none_none ${SOURCE_LIST_88})
set_source_files_properties(${SOURCE_LIST_88} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_11.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_12.dyn_gcc_32_none_none ${SOURCE_LIST_6})
set_source_files_properties(${SOURCE_LIST_6} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_12.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_13.dyn_gcc_32_none_none ${SOURCE_LIST_59})
set_source_files_properties(${SOURCE_LIST_59} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_13.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_14.dyn_gcc_32_none_none ${SOURCE_LIST_95})
set_source_files_properties(${SOURCE_LIST_95} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_14.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_5.dyn_gcc_32_none_none ${SOURCE_LIST_28})
set_source_files_properties(${SOURCE_LIST_28} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_5.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_6.dyn_gcc_32_none_none ${SOURCE_LIST_7})
set_source_files_properties(${SOURCE_LIST_7} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_6.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_7.dyn_gcc_32_none_none ${SOURCE_LIST_40})
set_source_files_properties(${SOURCE_LIST_40} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_7.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_8.dyn_gcc_32_none_none ${SOURCE_LIST_24})
set_source_files_properties(${SOURCE_LIST_24} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_8.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_9.dyn_gcc_32_none_none ${SOURCE_LIST_48})
set_source_files_properties(${SOURCE_LIST_48} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_9.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_1.dyn_gcc_32_none_none ${SOURCE_LIST_19})
set_source_files_properties(${SOURCE_LIST_19} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_1.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_2.dyn_gcc_32_none_none ${SOURCE_LIST_18})
set_source_files_properties(${SOURCE_LIST_18} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_2.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_3.dyn_gcc_32_none_none ${SOURCE_LIST_22})
set_source_files_properties(${SOURCE_LIST_22} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_3.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_4.dyn_gcc_32_none_none ${SOURCE_LIST_32})
set_source_files_properties(${SOURCE_LIST_32} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_4.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_5.dyn_gcc_32_none_none ${SOURCE_LIST_34})
set_source_files_properties(${SOURCE_LIST_34} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_5.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_6.dyn_gcc_32_none_none ${SOURCE_LIST_16})
set_source_files_properties(${SOURCE_LIST_16} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_6.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_7.dyn_gcc_32_none_none ${SOURCE_LIST_37})
set_source_files_properties(${SOURCE_LIST_37} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_7.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_mem_8.dyn_gcc_32_none_none ${SOURCE_LIST_9})
set_source_files_properties(${SOURCE_LIST_9} PROPERTIES LANGUAGE C)
set_target_properties(test_mem_8.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_snip_remove.dyn_gcc_32_none_none ${SOURCE_LIST_72})
set_source_files_properties(${SOURCE_LIST_72} PROPERTIES LANGUAGE C)
set_target_properties(test_snip_remove.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_1.dyn_gcc_32_none_none ${SOURCE_LIST_25})
set_source_files_properties(${SOURCE_LIST_25} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_1.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_2.dyn_gcc_32_none_none ${SOURCE_LIST_26})
set_source_files_properties(${SOURCE_LIST_26} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_2.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_3.dyn_gcc_32_none_none ${SOURCE_LIST_98})
set_source_files_properties(${SOURCE_LIST_98} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_3.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_4.dyn_gcc_32_none_none ${SOURCE_LIST_85})
set_source_files_properties(${SOURCE_LIST_85} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_4.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_thread_1.dyn_gcc_32_none_none ${SOURCE_LIST_15})
set_source_files_properties(${SOURCE_LIST_15} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_1.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_1.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_gcc_32_none_none ${SOURCE_LIST_90})
set_source_files_properties(${SOURCE_LIST_90} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_2.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_2.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_gcc_32_none_none ${SOURCE_LIST_61})
set_source_files_properties(${SOURCE_LIST_61} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_3.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_3.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_gcc_32_none_none ${SOURCE_LIST_86})
set_source_files_properties(${SOURCE_LIST_86} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_5.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_5.dyn_gcc_32_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_gcc_32_none_none ${SOURCE_LIST_36})
set_source_files_properties(${SOURCE_LIST_36} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_6.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_6.dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_gcc_32_none_none ${SOURCE_LIST_29})
set_source_files_properties(${SOURCE_LIST_29} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_7.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_7.dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_gcc_32_none_none ${SOURCE_LIST_5})
set_source_files_properties(${SOURCE_LIST_5} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_8.dyn_gcc_32_none_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_8.dyn_gcc_32_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_group_test.dyn_gcc_32_none_none
		init_fini_callback.dyn_gcc_32_none_none
		test1_12.dyn_gcc_32_none_none
		test1_14.dyn_gcc_32_none_none
		test1_19.dyn_gcc_32_none_none
		test1_29.dyn_gcc_32_none_none
		test1_35.dyn_gcc_32_none_none
		test1_40.dyn_gcc_32_none_none
		test1_41.dyn_gcc_32_none_none
		test2_14.dyn_gcc_32_none_none
		test2_6.dyn_gcc_32_none_none
		test2_8.dyn_gcc_32_none_none
		test3_1.dyn_gcc_32_none_none
		test3_2.dyn_gcc_32_none_none
		test3_3.dyn_gcc_32_none_none
		test3_4.dyn_gcc_32_none_none
		test3_5.dyn_gcc_32_none_none
		test3_6.dyn_gcc_32_none_none
		test3_7.dyn_gcc_32_none_none
		test4_1.dyn_gcc_32_none_none
		test4_2.dyn_gcc_32_none_none
		test4_3.dyn_gcc_32_none_none
		test4_3b.dyn_gcc_32_none_none
		test4_4.dyn_gcc_32_none_none
		test4_4b.dyn_gcc_32_none_none
		test_callback_1.dyn_gcc_32_none_none
		test_callback_2.dyn_gcc_32_none_none
		test_fork_10.dyn_gcc_32_none_none
		test_fork_11.dyn_gcc_32_none_none
		test_fork_12.dyn_gcc_32_none_none
		test_fork_13.dyn_gcc_32_none_none
		test_fork_14.dyn_gcc_32_none_none
		test_fork_5.dyn_gcc_32_none_none
		test_fork_6.dyn_gcc_32_none_none
		test_fork_7.dyn_gcc_32_none_none
		test_fork_8.dyn_gcc_32_none_none
		test_fork_9.dyn_gcc_32_none_none
		test_mem_1.dyn_gcc_32_none_none
		test_mem_2.dyn_gcc_32_none_none
		test_mem_3.dyn_gcc_32_none_none
		test_mem_4.dyn_gcc_32_none_none
		test_mem_5.dyn_gcc_32_none_none
		test_mem_6.dyn_gcc_32_none_none
		test_mem_7.dyn_gcc_32_none_none
		test_mem_8.dyn_gcc_32_none_none
		test_snip_remove.dyn_gcc_32_none_none
		test_stack_1.dyn_gcc_32_none_none
		test_stack_2.dyn_gcc_32_none_none
		test_stack_3.dyn_gcc_32_none_none
		test_stack_4.dyn_gcc_32_none_none
		test_thread_1.dyn_gcc_32_none_none
		test_thread_2.dyn_gcc_32_none_none
		test_thread_3.dyn_gcc_32_none_none
		test_thread_5.dyn_gcc_32_none_none
		test_thread_6.dyn_gcc_32_none_none
		test_thread_7.dyn_gcc_32_none_none
		test_thread_8.dyn_gcc_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cc not present")
ENDIF()
