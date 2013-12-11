# CMakeLists for dyninst/g++/32/stat/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  ${CXXFLAGS} -DSOLO_MUTATEE ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -static"
	"cxx"
	MUTATEEgnucxx32stat)

IF (MUTATEEgnucxx32stat MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-freebsd7.2/srclists.cmake)
add_executable (dyninst_cxx_group_test.stat_g++_32_pic_none ${SOURCE_LIST_68})
add_executable (dyninst_group_test.stat_g++_32_pic_none ${SOURCE_LIST_26})
add_executable (test1_14.stat_g++_32_pic_none ${SOURCE_LIST_48})
add_executable (test1_29.stat_g++_32_pic_none ${SOURCE_LIST_4})
add_executable (test1_35.stat_g++_32_pic_none ${SOURCE_LIST_65})


INSTALL (TARGETS
		dyninst_cxx_group_test.stat_g++_32_pic_none
		dyninst_group_test.stat_g++_32_pic_none
		test1_14.stat_g++_32_pic_none
		test1_29.stat_g++_32_pic_none
		test1_35.stat_g++_32_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
