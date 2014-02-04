# CMakeLists for symtab/g++/32/dyn/pic/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    -fPIC")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_gnu_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"CXX"
	"gnu_cxx 32 dyn"
	MUTATEEgnucxx32dynCXX)

IF (MUTATEEgnucxx32dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/ppc32_linux/srclists.cmake)
add_executable (symtab_group_test.dyn_g++_32_pic_none ${SOURCE_LIST_52})
set_source_files_properties(${SOURCE_LIST_52} PROPERTIES LANGUAGE CXX)
set_target_properties(symtab_group_test.dyn_g++_32_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (symtab_group_test.dyn_g++_32_pic_none testA_m32)


INSTALL (TARGETS
		symtab_group_test.dyn_g++_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
