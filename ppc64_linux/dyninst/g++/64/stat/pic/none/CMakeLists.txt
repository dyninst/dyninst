# CMakeLists for dyninst/g++/64/stat/pic/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}  -m64  -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_cxx}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_LDFLAGS_GNU} -m64 -static")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m64 -static")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}  -m64  -fPIC"
	"${MUTATEE_LDFLAGS_GNU} -m64 -static"
	"cxx"
	"gnu_cxx 64 stat"
	MUTATEEgnucxx64statcxx)

IF (MUTATEEgnucxx64statcxx MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc64_linux/srclists.cmake)
add_executable (dyninst_cxx_group_test.stat_g++_64_pic_none ${SOURCE_LIST_11})
add_executable (dyninst_group_test.stat_g++_64_pic_none ${SOURCE_LIST_14})
target_link_libraries (dyninst_group_test.stat_g++_64_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_g++_64_pic_none ${SOURCE_LIST_43})
add_executable (test1_29.stat_g++_64_pic_none ${SOURCE_LIST_4})


INSTALL (TARGETS
		dyninst_cxx_group_test.stat_g++_64_pic_none
		dyninst_group_test.stat_g++_64_pic_none
		test1_14.stat_g++_64_pic_none
		test1_29.stat_g++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
