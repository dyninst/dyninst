# CMakeLists for dyninst/bgq_g++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic")
IF (NOT ${M_mpi_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -dynamic -Wl,-export-dynamic"
	"cxx"
	"mpi_cxx 64 dyn"
	MUTATEEmpicxx64dyncxx)

IF (MUTATEEmpicxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_cxx_group_testdyn_bgq_g++_64_none_none ${SOURCE_LIST_46})
add_executable (dyninst_group_testdyn_bgq_g++_64_none_none ${SOURCE_LIST_29})
target_link_libraries (dyninst_group_testdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callbackdyn_bgq_g++_64_none_none ${SOURCE_LIST_47})
target_link_libraries (init_fini_callbackdyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12dyn_bgq_g++_64_none_none ${SOURCE_LIST_58})
add_executable (test1_14dyn_bgq_g++_64_none_none ${SOURCE_LIST_39})
add_executable (test1_19dyn_bgq_g++_64_none_none ${SOURCE_LIST_20})
add_executable (test1_29dyn_bgq_g++_64_none_none ${SOURCE_LIST_9})
add_executable (test1_41dyn_bgq_g++_64_none_none ${SOURCE_LIST_6})
add_executable (test2_14dyn_bgq_g++_64_none_none ${SOURCE_LIST_48})
add_executable (test2_6dyn_bgq_g++_64_none_none ${SOURCE_LIST_1})
target_link_libraries (test2_6dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS})
add_executable (test2_8dyn_bgq_g++_64_none_none ${SOURCE_LIST_2})
add_executable (test3_1dyn_bgq_g++_64_none_none ${SOURCE_LIST_26})
add_executable (test3_2dyn_bgq_g++_64_none_none ${SOURCE_LIST_31})
add_executable (test3_3dyn_bgq_g++_64_none_none ${SOURCE_LIST_44})
add_executable (test3_4dyn_bgq_g++_64_none_none ${SOURCE_LIST_51})
add_executable (test3_5dyn_bgq_g++_64_none_none ${SOURCE_LIST_40})
add_executable (test3_6dyn_bgq_g++_64_none_none ${SOURCE_LIST_0})
add_executable (test3_7dyn_bgq_g++_64_none_none ${SOURCE_LIST_5})
add_executable (test4_1dyn_bgq_g++_64_none_none ${SOURCE_LIST_3})
add_executable (test4_2dyn_bgq_g++_64_none_none ${SOURCE_LIST_57})
add_executable (test4_3dyn_bgq_g++_64_none_none ${SOURCE_LIST_53})
add_executable (test4_3bdyn_bgq_g++_64_none_none ${SOURCE_LIST_59})
add_executable (test4_4dyn_bgq_g++_64_none_none ${SOURCE_LIST_33})
add_executable (test4_4bdyn_bgq_g++_64_none_none ${SOURCE_LIST_49})
add_executable (test_callback_1dyn_bgq_g++_64_none_none ${SOURCE_LIST_14})
add_executable (test_callback_2dyn_bgq_g++_64_none_none ${SOURCE_LIST_24})
add_executable (test_fork_10dyn_bgq_g++_64_none_none ${SOURCE_LIST_41})
add_executable (test_fork_11dyn_bgq_g++_64_none_none ${SOURCE_LIST_17})
add_executable (test_fork_12dyn_bgq_g++_64_none_none ${SOURCE_LIST_22})
add_executable (test_fork_13dyn_bgq_g++_64_none_none ${SOURCE_LIST_10})
add_executable (test_fork_14dyn_bgq_g++_64_none_none ${SOURCE_LIST_18})
add_executable (test_fork_5dyn_bgq_g++_64_none_none ${SOURCE_LIST_54})
add_executable (test_fork_6dyn_bgq_g++_64_none_none ${SOURCE_LIST_42})
add_executable (test_fork_7dyn_bgq_g++_64_none_none ${SOURCE_LIST_21})
add_executable (test_fork_8dyn_bgq_g++_64_none_none ${SOURCE_LIST_15})
add_executable (test_fork_9dyn_bgq_g++_64_none_none ${SOURCE_LIST_32})
add_executable (test_snip_removedyn_bgq_g++_64_none_none ${SOURCE_LIST_43})
add_executable (test_stack_1dyn_bgq_g++_64_none_none ${SOURCE_LIST_38})
add_executable (test_stack_2dyn_bgq_g++_64_none_none ${SOURCE_LIST_50})
add_executable (test_stack_3dyn_bgq_g++_64_none_none ${SOURCE_LIST_35})
add_executable (test_stack_4dyn_bgq_g++_64_none_none ${SOURCE_LIST_55})
add_executable (test_thread_1dyn_bgq_g++_64_none_none ${SOURCE_LIST_28})
target_link_libraries (test_thread_1dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2dyn_bgq_g++_64_none_none ${SOURCE_LIST_8})
target_link_libraries (test_thread_2dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3dyn_bgq_g++_64_none_none ${SOURCE_LIST_13})
target_link_libraries (test_thread_3dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5dyn_bgq_g++_64_none_none ${SOURCE_LIST_12})
target_link_libraries (test_thread_5dyn_bgq_g++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6dyn_bgq_g++_64_none_none ${SOURCE_LIST_52})
target_link_libraries (test_thread_6dyn_bgq_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7dyn_bgq_g++_64_none_none ${SOURCE_LIST_56})
target_link_libraries (test_thread_7dyn_bgq_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8dyn_bgq_g++_64_none_none ${SOURCE_LIST_7})
target_link_libraries (test_thread_8dyn_bgq_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_cxx_group_testdyn_bgq_g++_64_none_none
		dyninst_group_testdyn_bgq_g++_64_none_none
		init_fini_callbackdyn_bgq_g++_64_none_none
		test1_12dyn_bgq_g++_64_none_none
		test1_14dyn_bgq_g++_64_none_none
		test1_19dyn_bgq_g++_64_none_none
		test1_29dyn_bgq_g++_64_none_none
		test1_41dyn_bgq_g++_64_none_none
		test2_14dyn_bgq_g++_64_none_none
		test2_6dyn_bgq_g++_64_none_none
		test2_8dyn_bgq_g++_64_none_none
		test3_1dyn_bgq_g++_64_none_none
		test3_2dyn_bgq_g++_64_none_none
		test3_3dyn_bgq_g++_64_none_none
		test3_4dyn_bgq_g++_64_none_none
		test3_5dyn_bgq_g++_64_none_none
		test3_6dyn_bgq_g++_64_none_none
		test3_7dyn_bgq_g++_64_none_none
		test4_1dyn_bgq_g++_64_none_none
		test4_2dyn_bgq_g++_64_none_none
		test4_3dyn_bgq_g++_64_none_none
		test4_3bdyn_bgq_g++_64_none_none
		test4_4dyn_bgq_g++_64_none_none
		test4_4bdyn_bgq_g++_64_none_none
		test_callback_1dyn_bgq_g++_64_none_none
		test_callback_2dyn_bgq_g++_64_none_none
		test_fork_10dyn_bgq_g++_64_none_none
		test_fork_11dyn_bgq_g++_64_none_none
		test_fork_12dyn_bgq_g++_64_none_none
		test_fork_13dyn_bgq_g++_64_none_none
		test_fork_14dyn_bgq_g++_64_none_none
		test_fork_5dyn_bgq_g++_64_none_none
		test_fork_6dyn_bgq_g++_64_none_none
		test_fork_7dyn_bgq_g++_64_none_none
		test_fork_8dyn_bgq_g++_64_none_none
		test_fork_9dyn_bgq_g++_64_none_none
		test_snip_removedyn_bgq_g++_64_none_none
		test_stack_1dyn_bgq_g++_64_none_none
		test_stack_2dyn_bgq_g++_64_none_none
		test_stack_3dyn_bgq_g++_64_none_none
		test_stack_4dyn_bgq_g++_64_none_none
		test_thread_1dyn_bgq_g++_64_none_none
		test_thread_2dyn_bgq_g++_64_none_none
		test_thread_3dyn_bgq_g++_64_none_none
		test_thread_5dyn_bgq_g++_64_none_none
		test_thread_6dyn_bgq_g++_64_none_none
		test_thread_7dyn_bgq_g++_64_none_none
		test_thread_8dyn_bgq_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_cxx not present")
ENDIF()
