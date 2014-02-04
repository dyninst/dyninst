# CMakeLists for dyninst/g++/32/stat/none/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_gnu_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -static")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -static"
	"CXX"
	"gnu_cxx 32 stat"
	MUTATEEgnucxx32statCXX)

IF (MUTATEEgnucxx32statCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-freebsd7.2/srclists.cmake)
add_executable (dyninst_cxx_group_test.stat_g++_32_none_none ${SOURCE_LIST_69})
set_source_files_properties(${SOURCE_LIST_69} PROPERTIES LANGUAGE CXX)
set_target_properties(dyninst_cxx_group_test.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (dyninst_group_test.stat_g++_32_none_none ${SOURCE_LIST_26})
set_source_files_properties(${SOURCE_LIST_26} PROPERTIES LANGUAGE CXX)
set_target_properties(dyninst_group_test.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_14.stat_g++_32_none_none ${SOURCE_LIST_48})
set_source_files_properties(${SOURCE_LIST_48} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_14.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_29.stat_g++_32_none_none ${SOURCE_LIST_4})
set_source_files_properties(${SOURCE_LIST_4} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_29.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)
add_executable (test1_35.stat_g++_32_none_none ${SOURCE_LIST_66})
set_source_files_properties(${SOURCE_LIST_66} PROPERTIES LANGUAGE CXX)
set_target_properties(test1_35.stat_g++_32_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		dyninst_cxx_group_test.stat_g++_32_none_none
		dyninst_group_test.stat_g++_32_none_none
		test1_14.stat_g++_32_none_none
		test1_29.stat_g++_32_none_none
		test1_35.stat_g++_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
