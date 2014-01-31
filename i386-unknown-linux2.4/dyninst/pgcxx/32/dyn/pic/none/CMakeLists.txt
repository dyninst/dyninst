# CMakeLists for dyninst/pgcxx/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}    -KPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_pg_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_pg_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_pg_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS_NATIVE} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_NATIVE}    -KPIC"
	"${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"cxx"
	"pg_cxx 32 dyn"
	MUTATEEpgcxx32dyncxx)

IF (MUTATEEpgcxx32dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_cxx_group_testdyn_pgcxx_32_pic_none ${SOURCE_LIST_92})
add_executable (dyninst_group_testdyn_pgcxx_32_pic_none ${SOURCE_LIST_69})
target_link_libraries (dyninst_group_testdyn_pgcxx_32_pic_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callbackdyn_pgcxx_32_pic_none ${SOURCE_LIST_74})
target_link_libraries (init_fini_callbackdyn_pgcxx_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_12dyn_pgcxx_32_pic_none ${SOURCE_LIST_78})
add_executable (test1_14dyn_pgcxx_32_pic_none ${SOURCE_LIST_70})
add_executable (test1_19dyn_pgcxx_32_pic_none ${SOURCE_LIST_39})
add_executable (test1_29dyn_pgcxx_32_pic_none ${SOURCE_LIST_58})
add_executable (test1_35dyn_pgcxx_32_pic_none ${SOURCE_LIST_91})
add_executable (test1_40dyn_pgcxx_32_pic_none ${SOURCE_LIST_47})
add_executable (test1_41dyn_pgcxx_32_pic_none ${SOURCE_LIST_83})
add_executable (test2_14dyn_pgcxx_32_pic_none ${SOURCE_LIST_75})
add_executable (test2_6dyn_pgcxx_32_pic_none ${SOURCE_LIST_0})
target_link_libraries (test2_6dyn_pgcxx_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test2_8dyn_pgcxx_32_pic_none ${SOURCE_LIST_64})
add_executable (test3_1dyn_pgcxx_32_pic_none ${SOURCE_LIST_43})
add_executable (test3_2dyn_pgcxx_32_pic_none ${SOURCE_LIST_46})
add_executable (test3_3dyn_pgcxx_32_pic_none ${SOURCE_LIST_96})
add_executable (test3_4dyn_pgcxx_32_pic_none ${SOURCE_LIST_54})
add_executable (test3_5dyn_pgcxx_32_pic_none ${SOURCE_LIST_94})
add_executable (test3_6dyn_pgcxx_32_pic_none ${SOURCE_LIST_33})
add_executable (test3_7dyn_pgcxx_32_pic_none ${SOURCE_LIST_82})
add_executable (test4_1dyn_pgcxx_32_pic_none ${SOURCE_LIST_1})
add_executable (test4_2dyn_pgcxx_32_pic_none ${SOURCE_LIST_56})
add_executable (test4_3dyn_pgcxx_32_pic_none ${SOURCE_LIST_102})
add_executable (test4_3bdyn_pgcxx_32_pic_none ${SOURCE_LIST_31})
add_executable (test4_4dyn_pgcxx_32_pic_none ${SOURCE_LIST_50})
add_executable (test4_4bdyn_pgcxx_32_pic_none ${SOURCE_LIST_67})
add_executable (test_callback_1dyn_pgcxx_32_pic_none ${SOURCE_LIST_62})
add_executable (test_callback_2dyn_pgcxx_32_pic_none ${SOURCE_LIST_66})
add_executable (test_fork_10dyn_pgcxx_32_pic_none ${SOURCE_LIST_71})
add_executable (test_fork_11dyn_pgcxx_32_pic_none ${SOURCE_LIST_88})
add_executable (test_fork_12dyn_pgcxx_32_pic_none ${SOURCE_LIST_6})
add_executable (test_fork_13dyn_pgcxx_32_pic_none ${SOURCE_LIST_59})
add_executable (test_fork_14dyn_pgcxx_32_pic_none ${SOURCE_LIST_95})
add_executable (test_fork_5dyn_pgcxx_32_pic_none ${SOURCE_LIST_28})
add_executable (test_fork_6dyn_pgcxx_32_pic_none ${SOURCE_LIST_7})
add_executable (test_fork_7dyn_pgcxx_32_pic_none ${SOURCE_LIST_40})
add_executable (test_fork_8dyn_pgcxx_32_pic_none ${SOURCE_LIST_24})
add_executable (test_fork_9dyn_pgcxx_32_pic_none ${SOURCE_LIST_48})
add_executable (test_mem_1dyn_pgcxx_32_pic_none ${SOURCE_LIST_19})
add_executable (test_mem_2dyn_pgcxx_32_pic_none ${SOURCE_LIST_18})
add_executable (test_mem_3dyn_pgcxx_32_pic_none ${SOURCE_LIST_22})
add_executable (test_mem_4dyn_pgcxx_32_pic_none ${SOURCE_LIST_32})
add_executable (test_mem_5dyn_pgcxx_32_pic_none ${SOURCE_LIST_34})
add_executable (test_mem_6dyn_pgcxx_32_pic_none ${SOURCE_LIST_16})
add_executable (test_mem_7dyn_pgcxx_32_pic_none ${SOURCE_LIST_37})
add_executable (test_mem_8dyn_pgcxx_32_pic_none ${SOURCE_LIST_9})
add_executable (test_snip_removedyn_pgcxx_32_pic_none ${SOURCE_LIST_72})
add_executable (test_stack_1dyn_pgcxx_32_pic_none ${SOURCE_LIST_25})
add_executable (test_stack_2dyn_pgcxx_32_pic_none ${SOURCE_LIST_26})
add_executable (test_stack_3dyn_pgcxx_32_pic_none ${SOURCE_LIST_98})
add_executable (test_stack_4dyn_pgcxx_32_pic_none ${SOURCE_LIST_85})
add_executable (test_thread_1dyn_pgcxx_32_pic_none ${SOURCE_LIST_15})
target_link_libraries (test_thread_1dyn_pgcxx_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2dyn_pgcxx_32_pic_none ${SOURCE_LIST_90})
target_link_libraries (test_thread_2dyn_pgcxx_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3dyn_pgcxx_32_pic_none ${SOURCE_LIST_61})
target_link_libraries (test_thread_3dyn_pgcxx_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5dyn_pgcxx_32_pic_none ${SOURCE_LIST_86})
target_link_libraries (test_thread_5dyn_pgcxx_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6dyn_pgcxx_32_pic_none ${SOURCE_LIST_36})
target_link_libraries (test_thread_6dyn_pgcxx_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7dyn_pgcxx_32_pic_none ${SOURCE_LIST_29})
target_link_libraries (test_thread_7dyn_pgcxx_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8dyn_pgcxx_32_pic_none ${SOURCE_LIST_5})
target_link_libraries (test_thread_8dyn_pgcxx_32_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_cxx_group_testdyn_pgcxx_32_pic_none
		dyninst_group_testdyn_pgcxx_32_pic_none
		init_fini_callbackdyn_pgcxx_32_pic_none
		test1_12dyn_pgcxx_32_pic_none
		test1_14dyn_pgcxx_32_pic_none
		test1_19dyn_pgcxx_32_pic_none
		test1_29dyn_pgcxx_32_pic_none
		test1_35dyn_pgcxx_32_pic_none
		test1_40dyn_pgcxx_32_pic_none
		test1_41dyn_pgcxx_32_pic_none
		test2_14dyn_pgcxx_32_pic_none
		test2_6dyn_pgcxx_32_pic_none
		test2_8dyn_pgcxx_32_pic_none
		test3_1dyn_pgcxx_32_pic_none
		test3_2dyn_pgcxx_32_pic_none
		test3_3dyn_pgcxx_32_pic_none
		test3_4dyn_pgcxx_32_pic_none
		test3_5dyn_pgcxx_32_pic_none
		test3_6dyn_pgcxx_32_pic_none
		test3_7dyn_pgcxx_32_pic_none
		test4_1dyn_pgcxx_32_pic_none
		test4_2dyn_pgcxx_32_pic_none
		test4_3dyn_pgcxx_32_pic_none
		test4_3bdyn_pgcxx_32_pic_none
		test4_4dyn_pgcxx_32_pic_none
		test4_4bdyn_pgcxx_32_pic_none
		test_callback_1dyn_pgcxx_32_pic_none
		test_callback_2dyn_pgcxx_32_pic_none
		test_fork_10dyn_pgcxx_32_pic_none
		test_fork_11dyn_pgcxx_32_pic_none
		test_fork_12dyn_pgcxx_32_pic_none
		test_fork_13dyn_pgcxx_32_pic_none
		test_fork_14dyn_pgcxx_32_pic_none
		test_fork_5dyn_pgcxx_32_pic_none
		test_fork_6dyn_pgcxx_32_pic_none
		test_fork_7dyn_pgcxx_32_pic_none
		test_fork_8dyn_pgcxx_32_pic_none
		test_fork_9dyn_pgcxx_32_pic_none
		test_mem_1dyn_pgcxx_32_pic_none
		test_mem_2dyn_pgcxx_32_pic_none
		test_mem_3dyn_pgcxx_32_pic_none
		test_mem_4dyn_pgcxx_32_pic_none
		test_mem_5dyn_pgcxx_32_pic_none
		test_mem_6dyn_pgcxx_32_pic_none
		test_mem_7dyn_pgcxx_32_pic_none
		test_mem_8dyn_pgcxx_32_pic_none
		test_snip_removedyn_pgcxx_32_pic_none
		test_stack_1dyn_pgcxx_32_pic_none
		test_stack_2dyn_pgcxx_32_pic_none
		test_stack_3dyn_pgcxx_32_pic_none
		test_stack_4dyn_pgcxx_32_pic_none
		test_thread_1dyn_pgcxx_32_pic_none
		test_thread_2dyn_pgcxx_32_pic_none
		test_thread_3dyn_pgcxx_32_pic_none
		test_thread_5dyn_pgcxx_32_pic_none
		test_thread_6dyn_pgcxx_32_pic_none
		test_thread_7dyn_pgcxx_32_pic_none
		test_thread_8dyn_pgcxx_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for pg_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler pg_cxx not present")
ENDIF()
