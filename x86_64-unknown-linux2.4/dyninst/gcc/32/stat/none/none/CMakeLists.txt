# CMakeLists for dyninst/gcc/32/stat/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}  -m32 -Di386_unknown_linux2_4 -Dm32_test  ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -static")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -static")
IF (NOT ${M_gnu_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cc}"
	"-g ${MUTATOR_DEFINES}  ${CFLAGS} ${MUTATEE_CFLAGS_GNU}  -m32 -Di386_unknown_linux2_4 -Dm32_test  "
	"${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -static"
	"c"
	"gnu_cc 32 stat"
	MUTATEEgnucc32statc)

IF (MUTATEEgnucc32statc MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_group_test.stat_gcc_32_none_none ${SOURCE_LIST_48})
target_link_libraries (dyninst_group_test.stat_gcc_32_none_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_gcc_32_none_none ${SOURCE_LIST_53})
add_executable (test1_29.stat_gcc_32_none_none ${SOURCE_LIST_4})


INSTALL (TARGETS
		dyninst_group_test.stat_gcc_32_none_none
		test1_14.stat_gcc_32_none_none
		test1_29.stat_gcc_32_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
