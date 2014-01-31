# CMakeLists for dyninst/g++/64/stat/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${MUTATEE_LDFLAGS_GNU}  -static")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -static"
	"cxx"
	"gnu_cxx 64 stat"
	MUTATEEgnucxx64statcxx)

IF (MUTATEEgnucxx64statcxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/amd64-unknown-freebsd7.2/srclists.cmake)
add_executable (dyninst_cxx_group_teststat_g++_64_pic_none ${SOURCE_LIST_65})
add_executable (dyninst_group_teststat_g++_64_pic_none ${SOURCE_LIST_70})
add_executable (test1_14stat_g++_64_pic_none ${SOURCE_LIST_50})
add_executable (test1_29stat_g++_64_pic_none ${SOURCE_LIST_4})
add_executable (test1_35stat_g++_64_pic_none ${SOURCE_LIST_42})


INSTALL (TARGETS
		dyninst_cxx_group_teststat_g++_64_pic_none
		dyninst_group_teststat_g++_64_pic_none
		test1_14stat_g++_64_pic_none
		test1_29stat_g++_64_pic_none
		test1_35stat_g++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
