# CMakeLists for dyninst/g++/64/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"cxx"
	"gnu_cxx 64 dyn"
	MUTATEEgnucxx64dyncxx)

IF (MUTATEEgnucxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/amd64-unknown-freebsd7.2/srclists.cmake)
add_executable (amd64_7_arg_calldyn_g++_64_pic_none ${SOURCE_LIST_45})
add_executable (dyninst_cxx_group_testdyn_g++_64_pic_none ${SOURCE_LIST_65})
add_executable (dyninst_group_testdyn_g++_64_pic_none ${SOURCE_LIST_70})
add_executable (init_fini_callbackdyn_g++_64_pic_none ${SOURCE_LIST_59})
add_executable (test1_12dyn_g++_64_pic_none ${SOURCE_LIST_71})
add_executable (test1_14dyn_g++_64_pic_none ${SOURCE_LIST_50})
add_executable (test1_19dyn_g++_64_pic_none ${SOURCE_LIST_25})
add_executable (test1_29dyn_g++_64_pic_none ${SOURCE_LIST_4})
add_executable (test1_35dyn_g++_64_pic_none ${SOURCE_LIST_42})
add_executable (test1_40dyn_g++_64_pic_none ${SOURCE_LIST_39})
add_executable (test1_41dyn_g++_64_pic_none ${SOURCE_LIST_7})
add_executable (test2_14dyn_g++_64_pic_none ${SOURCE_LIST_60})
add_executable (test2_6dyn_g++_64_pic_none ${SOURCE_LIST_1})
add_executable (test2_8dyn_g++_64_pic_none ${SOURCE_LIST_2})
add_executable (test3_1dyn_g++_64_pic_none ${SOURCE_LIST_33})
add_executable (test3_2dyn_g++_64_pic_none ${SOURCE_LIST_38})
add_executable (test3_3dyn_g++_64_pic_none ${SOURCE_LIST_55})
add_executable (test3_4dyn_g++_64_pic_none ${SOURCE_LIST_63})
add_executable (test3_5dyn_g++_64_pic_none ${SOURCE_LIST_51})
add_executable (test3_6dyn_g++_64_pic_none ${SOURCE_LIST_0})
add_executable (test3_7dyn_g++_64_pic_none ${SOURCE_LIST_29})
add_executable (test4_1dyn_g++_64_pic_none ${SOURCE_LIST_3})
add_executable (test4_2dyn_g++_64_pic_none ${SOURCE_LIST_58})
add_executable (test4_3dyn_g++_64_pic_none ${SOURCE_LIST_26})
add_executable (test4_3bdyn_g++_64_pic_none ${SOURCE_LIST_72})
add_executable (test4_4dyn_g++_64_pic_none ${SOURCE_LIST_43})
add_executable (test4_4bdyn_g++_64_pic_none ${SOURCE_LIST_61})
add_executable (test_callback_1dyn_g++_64_pic_none ${SOURCE_LIST_18})
add_executable (test_callback_2dyn_g++_64_pic_none ${SOURCE_LIST_31})
add_executable (test_fork_10dyn_g++_64_pic_none ${SOURCE_LIST_52})
add_executable (test_fork_11dyn_g++_64_pic_none ${SOURCE_LIST_21})
add_executable (test_fork_12dyn_g++_64_pic_none ${SOURCE_LIST_28})
add_executable (test_fork_13dyn_g++_64_pic_none ${SOURCE_LIST_14})
add_executable (test_fork_14dyn_g++_64_pic_none ${SOURCE_LIST_23})
add_executable (test_fork_5dyn_g++_64_pic_none ${SOURCE_LIST_66})
add_executable (test_fork_6dyn_g++_64_pic_none ${SOURCE_LIST_53})
add_executable (test_fork_7dyn_g++_64_pic_none ${SOURCE_LIST_9})
add_executable (test_fork_8dyn_g++_64_pic_none ${SOURCE_LIST_19})
add_executable (test_fork_9dyn_g++_64_pic_none ${SOURCE_LIST_41})
add_executable (test_mem_1dyn_g++_64_pic_none ${SOURCE_LIST_40})
add_executable (test_mem_2dyn_g++_64_pic_none ${SOURCE_LIST_6})
add_executable (test_mem_3dyn_g++_64_pic_none ${SOURCE_LIST_10})
add_executable (test_mem_4dyn_g++_64_pic_none ${SOURCE_LIST_69})
add_executable (test_mem_5dyn_g++_64_pic_none ${SOURCE_LIST_57})
add_executable (test_mem_6dyn_g++_64_pic_none ${SOURCE_LIST_27})
add_executable (test_mem_7dyn_g++_64_pic_none ${SOURCE_LIST_22})
add_executable (test_mem_8dyn_g++_64_pic_none ${SOURCE_LIST_36})
add_executable (test_snip_removedyn_g++_64_pic_none ${SOURCE_LIST_54})
add_executable (test_stack_1dyn_g++_64_pic_none ${SOURCE_LIST_49})
add_executable (test_stack_2dyn_g++_64_pic_none ${SOURCE_LIST_62})
add_executable (test_stack_3dyn_g++_64_pic_none ${SOURCE_LIST_46})
add_executable (test_stack_4dyn_g++_64_pic_none ${SOURCE_LIST_67})
add_executable (test_thread_1dyn_g++_64_pic_none ${SOURCE_LIST_35})
target_link_libraries (test_thread_1dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2dyn_g++_64_pic_none ${SOURCE_LIST_12})
target_link_libraries (test_thread_2dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3dyn_g++_64_pic_none ${SOURCE_LIST_17})
target_link_libraries (test_thread_3dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5dyn_g++_64_pic_none ${SOURCE_LIST_16})
target_link_libraries (test_thread_5dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6dyn_g++_64_pic_none ${SOURCE_LIST_64})
target_link_libraries (test_thread_6dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7dyn_g++_64_pic_none ${SOURCE_LIST_68})
target_link_libraries (test_thread_7dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8dyn_g++_64_pic_none ${SOURCE_LIST_11})
target_link_libraries (test_thread_8dyn_g++_64_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		amd64_7_arg_calldyn_g++_64_pic_none
		dyninst_cxx_group_testdyn_g++_64_pic_none
		dyninst_group_testdyn_g++_64_pic_none
		init_fini_callbackdyn_g++_64_pic_none
		test1_12dyn_g++_64_pic_none
		test1_14dyn_g++_64_pic_none
		test1_19dyn_g++_64_pic_none
		test1_29dyn_g++_64_pic_none
		test1_35dyn_g++_64_pic_none
		test1_40dyn_g++_64_pic_none
		test1_41dyn_g++_64_pic_none
		test2_14dyn_g++_64_pic_none
		test2_6dyn_g++_64_pic_none
		test2_8dyn_g++_64_pic_none
		test3_1dyn_g++_64_pic_none
		test3_2dyn_g++_64_pic_none
		test3_3dyn_g++_64_pic_none
		test3_4dyn_g++_64_pic_none
		test3_5dyn_g++_64_pic_none
		test3_6dyn_g++_64_pic_none
		test3_7dyn_g++_64_pic_none
		test4_1dyn_g++_64_pic_none
		test4_2dyn_g++_64_pic_none
		test4_3dyn_g++_64_pic_none
		test4_3bdyn_g++_64_pic_none
		test4_4dyn_g++_64_pic_none
		test4_4bdyn_g++_64_pic_none
		test_callback_1dyn_g++_64_pic_none
		test_callback_2dyn_g++_64_pic_none
		test_fork_10dyn_g++_64_pic_none
		test_fork_11dyn_g++_64_pic_none
		test_fork_12dyn_g++_64_pic_none
		test_fork_13dyn_g++_64_pic_none
		test_fork_14dyn_g++_64_pic_none
		test_fork_5dyn_g++_64_pic_none
		test_fork_6dyn_g++_64_pic_none
		test_fork_7dyn_g++_64_pic_none
		test_fork_8dyn_g++_64_pic_none
		test_fork_9dyn_g++_64_pic_none
		test_mem_1dyn_g++_64_pic_none
		test_mem_2dyn_g++_64_pic_none
		test_mem_3dyn_g++_64_pic_none
		test_mem_4dyn_g++_64_pic_none
		test_mem_5dyn_g++_64_pic_none
		test_mem_6dyn_g++_64_pic_none
		test_mem_7dyn_g++_64_pic_none
		test_mem_8dyn_g++_64_pic_none
		test_snip_removedyn_g++_64_pic_none
		test_stack_1dyn_g++_64_pic_none
		test_stack_2dyn_g++_64_pic_none
		test_stack_3dyn_g++_64_pic_none
		test_stack_4dyn_g++_64_pic_none
		test_thread_1dyn_g++_64_pic_none
		test_thread_2dyn_g++_64_pic_none
		test_thread_3dyn_g++_64_pic_none
		test_thread_5dyn_g++_64_pic_none
		test_thread_6dyn_g++_64_pic_none
		test_thread_7dyn_g++_64_pic_none
		test_thread_8dyn_g++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
