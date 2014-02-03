# CMakeLists for dyninst/bgxlc++/64/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_mpi_xlcxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink")
IF (NOT ${M_mpi_xlcxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_mpi_xlcxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${CXXFLAGS}   "
	"${MUTATEE_LDFLAGS_NATIVE}  -qnostaticlink"
	"cxx"
	"mpi_xlcxx 64 dyn"
	MUTATEEmpixlcxx64dyncxx)

IF (MUTATEEmpixlcxx64dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_bgq_ion/srclists.cmake)
add_executable (dyninst_cxx_group_test.dyn_bgxlc++_64_none_none ${SOURCE_LIST_46})
add_executable (dyninst_group_test.dyn_bgxlc++_64_none_none ${SOURCE_LIST_29})
target_link_libraries (dyninst_group_test.dyn_bgxlc++_64_none_none ${CMAKE_DL_LIBS})
add_executable (init_fini_callback.dyn_bgxlc++_64_none_none ${SOURCE_LIST_47})
target_link_libraries (init_fini_callback.dyn_bgxlc++_64_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12.dyn_bgxlc++_64_none_none ${SOURCE_LIST_58})
add_executable (test1_14.dyn_bgxlc++_64_none_none ${SOURCE_LIST_39})
add_executable (test1_19.dyn_bgxlc++_64_none_none ${SOURCE_LIST_20})
add_executable (test1_29.dyn_bgxlc++_64_none_none ${SOURCE_LIST_9})
add_executable (test1_41.dyn_bgxlc++_64_none_none ${SOURCE_LIST_6})
add_executable (test2_14.dyn_bgxlc++_64_none_none ${SOURCE_LIST_48})
add_executable (test2_6.dyn_bgxlc++_64_none_none ${SOURCE_LIST_1})
target_link_libraries (test2_6.dyn_bgxlc++_64_none_none ${CMAKE_DL_LIBS})
add_executable (test2_8.dyn_bgxlc++_64_none_none ${SOURCE_LIST_2})
add_executable (test3_1.dyn_bgxlc++_64_none_none ${SOURCE_LIST_26})
add_executable (test3_2.dyn_bgxlc++_64_none_none ${SOURCE_LIST_31})
add_executable (test3_3.dyn_bgxlc++_64_none_none ${SOURCE_LIST_44})
add_executable (test3_4.dyn_bgxlc++_64_none_none ${SOURCE_LIST_51})
add_executable (test3_5.dyn_bgxlc++_64_none_none ${SOURCE_LIST_40})
add_executable (test3_6.dyn_bgxlc++_64_none_none ${SOURCE_LIST_0})
add_executable (test3_7.dyn_bgxlc++_64_none_none ${SOURCE_LIST_5})
add_executable (test4_1.dyn_bgxlc++_64_none_none ${SOURCE_LIST_3})
add_executable (test4_2.dyn_bgxlc++_64_none_none ${SOURCE_LIST_57})
add_executable (test4_3.dyn_bgxlc++_64_none_none ${SOURCE_LIST_53})
add_executable (test4_3b.dyn_bgxlc++_64_none_none ${SOURCE_LIST_59})
add_executable (test4_4.dyn_bgxlc++_64_none_none ${SOURCE_LIST_33})
add_executable (test4_4b.dyn_bgxlc++_64_none_none ${SOURCE_LIST_49})
add_executable (test_callback_1.dyn_bgxlc++_64_none_none ${SOURCE_LIST_14})
add_executable (test_callback_2.dyn_bgxlc++_64_none_none ${SOURCE_LIST_24})
add_executable (test_fork_10.dyn_bgxlc++_64_none_none ${SOURCE_LIST_41})
add_executable (test_fork_11.dyn_bgxlc++_64_none_none ${SOURCE_LIST_17})
add_executable (test_fork_12.dyn_bgxlc++_64_none_none ${SOURCE_LIST_22})
add_executable (test_fork_13.dyn_bgxlc++_64_none_none ${SOURCE_LIST_10})
add_executable (test_fork_14.dyn_bgxlc++_64_none_none ${SOURCE_LIST_18})
add_executable (test_fork_5.dyn_bgxlc++_64_none_none ${SOURCE_LIST_54})
add_executable (test_fork_6.dyn_bgxlc++_64_none_none ${SOURCE_LIST_42})
add_executable (test_fork_7.dyn_bgxlc++_64_none_none ${SOURCE_LIST_21})
add_executable (test_fork_8.dyn_bgxlc++_64_none_none ${SOURCE_LIST_15})
add_executable (test_fork_9.dyn_bgxlc++_64_none_none ${SOURCE_LIST_32})
add_executable (test_snip_remove.dyn_bgxlc++_64_none_none ${SOURCE_LIST_43})
add_executable (test_stack_1.dyn_bgxlc++_64_none_none ${SOURCE_LIST_38})
add_executable (test_stack_2.dyn_bgxlc++_64_none_none ${SOURCE_LIST_50})
add_executable (test_stack_3.dyn_bgxlc++_64_none_none ${SOURCE_LIST_35})
add_executable (test_stack_4.dyn_bgxlc++_64_none_none ${SOURCE_LIST_55})
add_executable (test_thread_1.dyn_bgxlc++_64_none_none ${SOURCE_LIST_28})
target_link_libraries (test_thread_1.dyn_bgxlc++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_2.dyn_bgxlc++_64_none_none ${SOURCE_LIST_8})
target_link_libraries (test_thread_2.dyn_bgxlc++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_3.dyn_bgxlc++_64_none_none ${SOURCE_LIST_13})
target_link_libraries (test_thread_3.dyn_bgxlc++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_5.dyn_bgxlc++_64_none_none ${SOURCE_LIST_12})
target_link_libraries (test_thread_5.dyn_bgxlc++_64_none_none ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_6.dyn_bgxlc++_64_none_none ${SOURCE_LIST_52})
target_link_libraries (test_thread_6.dyn_bgxlc++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_7.dyn_bgxlc++_64_none_none ${SOURCE_LIST_56})
target_link_libraries (test_thread_7.dyn_bgxlc++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (test_thread_8.dyn_bgxlc++_64_none_none ${SOURCE_LIST_7})
target_link_libraries (test_thread_8.dyn_bgxlc++_64_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		dyninst_cxx_group_test.dyn_bgxlc++_64_none_none
		dyninst_group_test.dyn_bgxlc++_64_none_none
		init_fini_callback.dyn_bgxlc++_64_none_none
		test1_12.dyn_bgxlc++_64_none_none
		test1_14.dyn_bgxlc++_64_none_none
		test1_19.dyn_bgxlc++_64_none_none
		test1_29.dyn_bgxlc++_64_none_none
		test1_41.dyn_bgxlc++_64_none_none
		test2_14.dyn_bgxlc++_64_none_none
		test2_6.dyn_bgxlc++_64_none_none
		test2_8.dyn_bgxlc++_64_none_none
		test3_1.dyn_bgxlc++_64_none_none
		test3_2.dyn_bgxlc++_64_none_none
		test3_3.dyn_bgxlc++_64_none_none
		test3_4.dyn_bgxlc++_64_none_none
		test3_5.dyn_bgxlc++_64_none_none
		test3_6.dyn_bgxlc++_64_none_none
		test3_7.dyn_bgxlc++_64_none_none
		test4_1.dyn_bgxlc++_64_none_none
		test4_2.dyn_bgxlc++_64_none_none
		test4_3.dyn_bgxlc++_64_none_none
		test4_3b.dyn_bgxlc++_64_none_none
		test4_4.dyn_bgxlc++_64_none_none
		test4_4b.dyn_bgxlc++_64_none_none
		test_callback_1.dyn_bgxlc++_64_none_none
		test_callback_2.dyn_bgxlc++_64_none_none
		test_fork_10.dyn_bgxlc++_64_none_none
		test_fork_11.dyn_bgxlc++_64_none_none
		test_fork_12.dyn_bgxlc++_64_none_none
		test_fork_13.dyn_bgxlc++_64_none_none
		test_fork_14.dyn_bgxlc++_64_none_none
		test_fork_5.dyn_bgxlc++_64_none_none
		test_fork_6.dyn_bgxlc++_64_none_none
		test_fork_7.dyn_bgxlc++_64_none_none
		test_fork_8.dyn_bgxlc++_64_none_none
		test_fork_9.dyn_bgxlc++_64_none_none
		test_snip_remove.dyn_bgxlc++_64_none_none
		test_stack_1.dyn_bgxlc++_64_none_none
		test_stack_2.dyn_bgxlc++_64_none_none
		test_stack_3.dyn_bgxlc++_64_none_none
		test_stack_4.dyn_bgxlc++_64_none_none
		test_thread_1.dyn_bgxlc++_64_none_none
		test_thread_2.dyn_bgxlc++_64_none_none
		test_thread_3.dyn_bgxlc++_64_none_none
		test_thread_5.dyn_bgxlc++_64_none_none
		test_thread_6.dyn_bgxlc++_64_none_none
		test_thread_7.dyn_bgxlc++_64_none_none
		test_thread_8.dyn_bgxlc++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for mpi_xlcxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler mpi_xlcxx not present")
ENDIF()
