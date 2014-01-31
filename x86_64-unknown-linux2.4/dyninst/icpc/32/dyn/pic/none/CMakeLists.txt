# CMakeLists for dyninst/icpc/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}   -Di386_unknown_linux2_4 -Dm32_test  -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_intel_CC_32}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic")
IF (NOT ${M_intel_CC_32} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_intel_CC_32}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}   -Di386_unknown_linux2_4 -Dm32_test  -fPIC"
	"${MUTATEE_LDFLAGS_GNU} -Di386_unknown_linux2_4 -Dm32_test -Xlinker -export-dynamic"
	"cxx"
	"intel_CC_32 32 dyn"
	MUTATEEintelCC3232dyncxx)

IF (MUTATEEintelCC3232dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (amd64_7_arg_calldyn_icpc_32_pic_none ${SOURCE_LIST_47})
add_executable (dyninst_group_testdyn_icpc_32_pic_none ${SOURCE_LIST_48})
target_link_libraries (dyninst_group_testdyn_icpc_32_pic_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callbackdyn_icpc_32_pic_none ${SOURCE_LIST_64})
target_link_libraries (init_fini_callbackdyn_icpc_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_12dyn_icpc_32_pic_none ${SOURCE_LIST_77})
add_executable (test1_14dyn_icpc_32_pic_none ${SOURCE_LIST_53})
add_executable (test1_19dyn_icpc_32_pic_none ${SOURCE_LIST_26})
add_executable (test1_29dyn_icpc_32_pic_none ${SOURCE_LIST_4})
add_executable (test1_40dyn_icpc_32_pic_none ${SOURCE_LIST_41})
add_executable (test1_41dyn_icpc_32_pic_none ${SOURCE_LIST_8})
add_executable (test2_14dyn_icpc_32_pic_none ${SOURCE_LIST_65})
add_executable (test2_6dyn_icpc_32_pic_none ${SOURCE_LIST_1})
target_link_libraries (test2_6dyn_icpc_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test2_8dyn_icpc_32_pic_none ${SOURCE_LIST_2})
add_executable (test3_1dyn_icpc_32_pic_none ${SOURCE_LIST_25})
add_executable (test3_2dyn_icpc_32_pic_none ${SOURCE_LIST_40})
add_executable (test3_3dyn_icpc_32_pic_none ${SOURCE_LIST_60})
add_executable (test3_4dyn_icpc_32_pic_none ${SOURCE_LIST_68})
add_executable (test3_5dyn_icpc_32_pic_none ${SOURCE_LIST_55})
add_executable (test3_6dyn_icpc_32_pic_none ${SOURCE_LIST_58})
add_executable (test3_7dyn_icpc_32_pic_none ${SOURCE_LIST_31})
add_executable (test4_1dyn_icpc_32_pic_none ${SOURCE_LIST_3})
add_executable (test4_2dyn_icpc_32_pic_none ${SOURCE_LIST_76})
add_executable (test4_3dyn_icpc_32_pic_none ${SOURCE_LIST_71})
add_executable (test4_3bdyn_icpc_32_pic_none ${SOURCE_LIST_78})
add_executable (test4_4dyn_icpc_32_pic_none ${SOURCE_LIST_45})
add_executable (test4_4bdyn_icpc_32_pic_none ${SOURCE_LIST_66})
add_executable (test_callback_1dyn_icpc_32_pic_none ${SOURCE_LIST_18})
add_executable (test_callback_2dyn_icpc_32_pic_none ${SOURCE_LIST_33})
add_executable (test_fork_10dyn_icpc_32_pic_none ${SOURCE_LIST_56})
add_executable (test_fork_11dyn_icpc_32_pic_none ${SOURCE_LIST_21})
add_executable (test_fork_12dyn_icpc_32_pic_none ${SOURCE_LIST_30})
add_executable (test_fork_13dyn_icpc_32_pic_none ${SOURCE_LIST_14})
add_executable (test_fork_14dyn_icpc_32_pic_none ${SOURCE_LIST_23})
add_executable (test_fork_5dyn_icpc_32_pic_none ${SOURCE_LIST_72})
add_executable (test_fork_6dyn_icpc_32_pic_none ${SOURCE_LIST_57})
add_executable (test_fork_7dyn_icpc_32_pic_none ${SOURCE_LIST_27})
add_executable (test_fork_8dyn_icpc_32_pic_none ${SOURCE_LIST_19})
add_executable (test_fork_9dyn_icpc_32_pic_none ${SOURCE_LIST_43})
add_executable (test_snip_removedyn_icpc_32_pic_none ${SOURCE_LIST_59})
add_executable (test_stack_1dyn_icpc_32_pic_none ${SOURCE_LIST_52})
add_executable (test_stack_2dyn_icpc_32_pic_none ${SOURCE_LIST_67})
add_executable (test_stack_3dyn_icpc_32_pic_none ${SOURCE_LIST_5})
add_executable (test_stack_4dyn_icpc_32_pic_none ${SOURCE_LIST_73})
add_executable (test_thread_1dyn_icpc_32_pic_none ${SOURCE_LIST_37})
target_link_libraries (test_thread_1dyn_icpc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2dyn_icpc_32_pic_none ${SOURCE_LIST_12})
target_link_libraries (test_thread_2dyn_icpc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3dyn_icpc_32_pic_none ${SOURCE_LIST_17})
target_link_libraries (test_thread_3dyn_icpc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5dyn_icpc_32_pic_none ${SOURCE_LIST_16})
target_link_libraries (test_thread_5dyn_icpc_32_pic_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6dyn_icpc_32_pic_none ${SOURCE_LIST_69})
target_link_libraries (test_thread_6dyn_icpc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7dyn_icpc_32_pic_none ${SOURCE_LIST_74})
target_link_libraries (test_thread_7dyn_icpc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8dyn_icpc_32_pic_none ${SOURCE_LIST_11})
target_link_libraries (test_thread_8dyn_icpc_32_pic_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		amd64_7_arg_calldyn_icpc_32_pic_none
		dyninst_group_testdyn_icpc_32_pic_none
		init_fini_callbackdyn_icpc_32_pic_none
		test1_12dyn_icpc_32_pic_none
		test1_14dyn_icpc_32_pic_none
		test1_19dyn_icpc_32_pic_none
		test1_29dyn_icpc_32_pic_none
		test1_40dyn_icpc_32_pic_none
		test1_41dyn_icpc_32_pic_none
		test2_14dyn_icpc_32_pic_none
		test2_6dyn_icpc_32_pic_none
		test2_8dyn_icpc_32_pic_none
		test3_1dyn_icpc_32_pic_none
		test3_2dyn_icpc_32_pic_none
		test3_3dyn_icpc_32_pic_none
		test3_4dyn_icpc_32_pic_none
		test3_5dyn_icpc_32_pic_none
		test3_6dyn_icpc_32_pic_none
		test3_7dyn_icpc_32_pic_none
		test4_1dyn_icpc_32_pic_none
		test4_2dyn_icpc_32_pic_none
		test4_3dyn_icpc_32_pic_none
		test4_3bdyn_icpc_32_pic_none
		test4_4dyn_icpc_32_pic_none
		test4_4bdyn_icpc_32_pic_none
		test_callback_1dyn_icpc_32_pic_none
		test_callback_2dyn_icpc_32_pic_none
		test_fork_10dyn_icpc_32_pic_none
		test_fork_11dyn_icpc_32_pic_none
		test_fork_12dyn_icpc_32_pic_none
		test_fork_13dyn_icpc_32_pic_none
		test_fork_14dyn_icpc_32_pic_none
		test_fork_5dyn_icpc_32_pic_none
		test_fork_6dyn_icpc_32_pic_none
		test_fork_7dyn_icpc_32_pic_none
		test_fork_8dyn_icpc_32_pic_none
		test_fork_9dyn_icpc_32_pic_none
		test_snip_removedyn_icpc_32_pic_none
		test_stack_1dyn_icpc_32_pic_none
		test_stack_2dyn_icpc_32_pic_none
		test_stack_3dyn_icpc_32_pic_none
		test_stack_4dyn_icpc_32_pic_none
		test_thread_1dyn_icpc_32_pic_none
		test_thread_2dyn_icpc_32_pic_none
		test_thread_3dyn_icpc_32_pic_none
		test_thread_5dyn_icpc_32_pic_none
		test_thread_6dyn_icpc_32_pic_none
		test_thread_7dyn_icpc_32_pic_none
		test_thread_8dyn_icpc_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for intel_CC_32 failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler intel_CC_32 not present")
ENDIF()
