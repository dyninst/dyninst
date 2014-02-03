# CMakeLists for dyninst/VC++/32/dyn/none/none
set (CMAKE_C_FLAGS "/D_DEBUG ${MUTATOR_DEFINES}  /TP ${CXXFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_native_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  ")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_native_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_native_cc}"
	"/D_DEBUG ${MUTATOR_DEFINES}  /TP ${CXXFLAGS}   "
	"${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}  "
	"cxx"
	"native_cc 32 dyn"
	MUTATEEnativecc32dyncxx)

IF (MUTATEEnativecc32dyncxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
add_executable (dyninst_cxx_group_testdyn_VC++_32_none_none ${SOURCE_LIST_22})
add_executable (dyninst_group_testdyn_VC++_32_none_none ${SOURCE_LIST_41})
target_link_libraries (dyninst_group_testdyn_VC++_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_12dyn_VC++_32_none_none ${SOURCE_LIST_42})
add_executable (test1_14dyn_VC++_32_none_none ${SOURCE_LIST_30})
add_executable (test1_19dyn_VC++_32_none_none ${SOURCE_LIST_14})
add_executable (test1_29dyn_VC++_32_none_none ${SOURCE_LIST_4})
add_executable (test2_14dyn_VC++_32_none_none ${SOURCE_LIST_35})
add_executable (test2_8dyn_VC++_32_none_none ${SOURCE_LIST_2})
add_executable (test3_1dyn_VC++_32_none_none ${SOURCE_LIST_20})
add_executable (test3_2dyn_VC++_32_none_none ${SOURCE_LIST_24})
add_executable (test3_3dyn_VC++_32_none_none ${SOURCE_LIST_32})
add_executable (test3_4dyn_VC++_32_none_none ${SOURCE_LIST_37})
add_executable (test3_7dyn_VC++_32_none_none ${SOURCE_LIST_16})
add_executable (test4_1dyn_VC++_32_none_none ${SOURCE_LIST_3})
add_executable (test_mem_1dyn_VC++_32_none_none ${SOURCE_LIST_40})
add_executable (test_mem_2dyn_VC++_32_none_none ${SOURCE_LIST_1})
add_executable (test_mem_3dyn_VC++_32_none_none ${SOURCE_LIST_15})
add_executable (test_mem_4dyn_VC++_32_none_none ${SOURCE_LIST_10})
add_executable (test_mem_5dyn_VC++_32_none_none ${SOURCE_LIST_18})
add_executable (test_mem_6dyn_VC++_32_none_none ${SOURCE_LIST_39})
add_executable (test_mem_7dyn_VC++_32_none_none ${SOURCE_LIST_7})
add_executable (test_mem_8dyn_VC++_32_none_none ${SOURCE_LIST_8})
add_executable (test_snip_removedyn_VC++_32_none_none ${SOURCE_LIST_31})
add_executable (test_stack_1dyn_VC++_32_none_none ${SOURCE_LIST_36})
add_executable (test_stack_3dyn_VC++_32_none_none ${SOURCE_LIST_34})
add_executable (test_thread_6dyn_VC++_32_none_none ${SOURCE_LIST_38})
add_executable (test_thread_7dyn_VC++_32_none_none ${SOURCE_LIST_26})
add_executable (test_thread_8dyn_VC++_32_none_none ${SOURCE_LIST_6})


INSTALL (TARGETS
		dyninst_cxx_group_testdyn_VC++_32_none_none
		dyninst_group_testdyn_VC++_32_none_none
		test1_12dyn_VC++_32_none_none
		test1_14dyn_VC++_32_none_none
		test1_19dyn_VC++_32_none_none
		test1_29dyn_VC++_32_none_none
		test2_14dyn_VC++_32_none_none
		test2_8dyn_VC++_32_none_none
		test3_1dyn_VC++_32_none_none
		test3_2dyn_VC++_32_none_none
		test3_3dyn_VC++_32_none_none
		test3_4dyn_VC++_32_none_none
		test3_7dyn_VC++_32_none_none
		test4_1dyn_VC++_32_none_none
		test_mem_1dyn_VC++_32_none_none
		test_mem_2dyn_VC++_32_none_none
		test_mem_3dyn_VC++_32_none_none
		test_mem_4dyn_VC++_32_none_none
		test_mem_5dyn_VC++_32_none_none
		test_mem_6dyn_VC++_32_none_none
		test_mem_7dyn_VC++_32_none_none
		test_mem_8dyn_VC++_32_none_none
		test_snip_removedyn_VC++_32_none_none
		test_stack_1dyn_VC++_32_none_none
		test_stack_3dyn_VC++_32_none_none
		test_thread_6dyn_VC++_32_none_none
		test_thread_7dyn_VC++_32_none_none
		test_thread_8dyn_VC++_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for native_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler native_cc not present")
ENDIF()
