# CMakeLists for dyninst/bgxlc/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}   -qpic")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_xlc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
IF (NOT ${M_mpi_xlc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${CFLAGS}   -qpic"
	"${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink"
	"C"
	"mpi_xlc 64 dyn"
	MUTATEEmpixlc64dynC)

IF (MUTATEEmpixlc64dynC MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_group_test.dyn_bgxlc_64_pic_none ${SOURCE_LIST_29})
set_source_files_properties(${SOURCE_LIST_29} PROPERTIES LANGUAGE C)
set_target_properties(dyninst_group_test.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (dyninst_group_test.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_bgxlc_64_pic_none ${SOURCE_LIST_47})
set_source_files_properties(${SOURCE_LIST_47} PROPERTIES LANGUAGE C)
set_target_properties(init_fini_callback.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (init_fini_callback.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_bgxlc_64_pic_none ${SOURCE_LIST_58})
set_source_files_properties(${SOURCE_LIST_58} PROPERTIES LANGUAGE C)
set_target_properties(test1_12.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_14.dyn_bgxlc_64_pic_none ${SOURCE_LIST_39})
set_source_files_properties(${SOURCE_LIST_39} PROPERTIES LANGUAGE C)
set_target_properties(test1_14.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_19.dyn_bgxlc_64_pic_none ${SOURCE_LIST_20})
set_source_files_properties(${SOURCE_LIST_20} PROPERTIES LANGUAGE C)
set_target_properties(test1_19.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_29.dyn_bgxlc_64_pic_none ${SOURCE_LIST_9})
set_source_files_properties(${SOURCE_LIST_9} PROPERTIES LANGUAGE C)
set_target_properties(test1_29.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test1_41.dyn_bgxlc_64_pic_none ${SOURCE_LIST_6})
set_source_files_properties(${SOURCE_LIST_6} PROPERTIES LANGUAGE C)
set_target_properties(test1_41.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_14.dyn_bgxlc_64_pic_none ${SOURCE_LIST_48})
set_source_files_properties(${SOURCE_LIST_48} PROPERTIES LANGUAGE C)
set_target_properties(test2_14.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test2_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_1})
set_source_files_properties(${SOURCE_LIST_1} PROPERTIES LANGUAGE C)
set_target_properties(test2_6.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test2_6.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_bgxlc_64_pic_none ${SOURCE_LIST_2})
set_source_files_properties(${SOURCE_LIST_2} PROPERTIES LANGUAGE C)
set_target_properties(test2_8.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_26})
set_source_files_properties(${SOURCE_LIST_26} PROPERTIES LANGUAGE C)
set_target_properties(test3_1.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_31})
set_source_files_properties(${SOURCE_LIST_31} PROPERTIES LANGUAGE C)
set_target_properties(test3_2.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_44})
set_source_files_properties(${SOURCE_LIST_44} PROPERTIES LANGUAGE C)
set_target_properties(test3_3.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_4.dyn_bgxlc_64_pic_none ${SOURCE_LIST_51})
set_source_files_properties(${SOURCE_LIST_51} PROPERTIES LANGUAGE C)
set_target_properties(test3_4.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_5.dyn_bgxlc_64_pic_none ${SOURCE_LIST_40})
set_source_files_properties(${SOURCE_LIST_40} PROPERTIES LANGUAGE C)
set_target_properties(test3_5.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_0})
set_source_files_properties(${SOURCE_LIST_0} PROPERTIES LANGUAGE C)
set_target_properties(test3_6.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test3_7.dyn_bgxlc_64_pic_none ${SOURCE_LIST_5})
set_source_files_properties(${SOURCE_LIST_5} PROPERTIES LANGUAGE C)
set_target_properties(test3_7.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_3})
set_source_files_properties(${SOURCE_LIST_3} PROPERTIES LANGUAGE C)
set_target_properties(test4_1.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_57})
set_source_files_properties(${SOURCE_LIST_57} PROPERTIES LANGUAGE C)
set_target_properties(test4_2.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_53})
set_source_files_properties(${SOURCE_LIST_53} PROPERTIES LANGUAGE C)
set_target_properties(test4_3.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_3b.dyn_bgxlc_64_pic_none ${SOURCE_LIST_59})
set_source_files_properties(${SOURCE_LIST_59} PROPERTIES LANGUAGE C)
set_target_properties(test4_3b.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4.dyn_bgxlc_64_pic_none ${SOURCE_LIST_33})
set_source_files_properties(${SOURCE_LIST_33} PROPERTIES LANGUAGE C)
set_target_properties(test4_4.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test4_4b.dyn_bgxlc_64_pic_none ${SOURCE_LIST_49})
set_source_files_properties(${SOURCE_LIST_49} PROPERTIES LANGUAGE C)
set_target_properties(test4_4b.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_14})
set_source_files_properties(${SOURCE_LIST_14} PROPERTIES LANGUAGE C)
set_target_properties(test_callback_1.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_callback_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_24})
set_source_files_properties(${SOURCE_LIST_24} PROPERTIES LANGUAGE C)
set_target_properties(test_callback_2.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_10.dyn_bgxlc_64_pic_none ${SOURCE_LIST_41})
set_source_files_properties(${SOURCE_LIST_41} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_10.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_11.dyn_bgxlc_64_pic_none ${SOURCE_LIST_17})
set_source_files_properties(${SOURCE_LIST_17} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_11.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_12.dyn_bgxlc_64_pic_none ${SOURCE_LIST_22})
set_source_files_properties(${SOURCE_LIST_22} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_12.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_13.dyn_bgxlc_64_pic_none ${SOURCE_LIST_10})
set_source_files_properties(${SOURCE_LIST_10} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_13.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_14.dyn_bgxlc_64_pic_none ${SOURCE_LIST_18})
set_source_files_properties(${SOURCE_LIST_18} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_14.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_5.dyn_bgxlc_64_pic_none ${SOURCE_LIST_54})
set_source_files_properties(${SOURCE_LIST_54} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_5.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_42})
set_source_files_properties(${SOURCE_LIST_42} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_6.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_7.dyn_bgxlc_64_pic_none ${SOURCE_LIST_21})
set_source_files_properties(${SOURCE_LIST_21} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_7.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_8.dyn_bgxlc_64_pic_none ${SOURCE_LIST_15})
set_source_files_properties(${SOURCE_LIST_15} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_8.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_fork_9.dyn_bgxlc_64_pic_none ${SOURCE_LIST_32})
set_source_files_properties(${SOURCE_LIST_32} PROPERTIES LANGUAGE C)
set_target_properties(test_fork_9.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_snip_remove.dyn_bgxlc_64_pic_none ${SOURCE_LIST_43})
set_source_files_properties(${SOURCE_LIST_43} PROPERTIES LANGUAGE C)
set_target_properties(test_snip_remove.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_38})
set_source_files_properties(${SOURCE_LIST_38} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_1.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_50})
set_source_files_properties(${SOURCE_LIST_50} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_2.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_35})
set_source_files_properties(${SOURCE_LIST_35} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_3.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_stack_4.dyn_bgxlc_64_pic_none ${SOURCE_LIST_55})
set_source_files_properties(${SOURCE_LIST_55} PROPERTIES LANGUAGE C)
set_target_properties(test_stack_4.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
add_executable (test_thread_1.dyn_bgxlc_64_pic_none ${SOURCE_LIST_28})
set_source_files_properties(${SOURCE_LIST_28} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_1.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_1.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_bgxlc_64_pic_none ${SOURCE_LIST_8})
set_source_files_properties(${SOURCE_LIST_8} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_2.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_2.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_bgxlc_64_pic_none ${SOURCE_LIST_13})
set_source_files_properties(${SOURCE_LIST_13} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_3.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_3.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_bgxlc_64_pic_none ${SOURCE_LIST_12})
set_source_files_properties(${SOURCE_LIST_12} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_5.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_5.dyn_bgxlc_64_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_bgxlc_64_pic_none ${SOURCE_LIST_52})
set_source_files_properties(${SOURCE_LIST_52} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_6.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_6.dyn_bgxlc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_bgxlc_64_pic_none ${SOURCE_LIST_56})
set_source_files_properties(${SOURCE_LIST_56} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_7.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
target_link_libraries (test_thread_7.dyn_bgxlc_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_bgxlc_64_pic_none ${SOURCE_LIST_7})
set_source_files_properties(${SOURCE_LIST_7} PROPERTIES LANGUAGE C)
set_target_properties(test_thread_8.dyn_bgxlc_64_pic_none PROPERTIES LINKER_LANGUAGE C)
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
		test_fork_14.dyn_bgxlc_64_pic_none
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

ELSE()
MESSAGE (STATUS "Compiler check for mpi_xlc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_xlc not present")
ENDIF()
