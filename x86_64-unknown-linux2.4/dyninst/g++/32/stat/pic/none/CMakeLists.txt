# CMakeLists for dyninst/g++/32/stat/pic/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}  -m32 -Di386_unknown_linux2_4 -Dm32_test  -fPIC")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_gnu_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -static")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}  -m32 -Di386_unknown_linux2_4 -Dm32_test  -fPIC"
	"${MUTATEE_LDFLAGS_GNU} -m32 -Di386_unknown_linux2_4 -Dm32_test -static"
	"CXX"
	"gnu_cxx 32 stat"
	MUTATEEgnucxx32statCXX)

IF (MUTATEEgnucxx32statCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
add_executable (dyninst_group_test.stat_g++_32_pic_none ${SOURCE_LIST_48})
set_source_files_properties(${SOURCE_LIST_48} PROPERTIES LANGUAGE CXX)
set_target_properties(dyninst_group_test.stat_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (dyninst_group_test.stat_g++_32_pic_none ${CMAKE_DL_LIBS})
add_executable (test1_14.stat_g++_32_pic_none ${SOURCE_LIST_53})
set_source_files_properties(${SOURCE_LIST_53} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_14.stat_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_29.stat_g++_32_pic_none ${SOURCE_LIST_4})
set_source_files_properties(${SOURCE_LIST_4} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_29.stat_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		dyninst_group_test.stat_g++_32_pic_none
		test1_14.stat_g++_32_pic_none
		test1_29.stat_g++_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
