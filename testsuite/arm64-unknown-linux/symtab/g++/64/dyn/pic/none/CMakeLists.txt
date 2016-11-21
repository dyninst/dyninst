# CMakeLists for symtab/g++/64/dyn/pic/none
set (CMAKE_CXX_FLAGS "${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}   -g -O0 -fPIC")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set (CMAKE_CXX_COMPILER "${M_gnu_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/symtab")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}   -g -O0 -fPIC"
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"CXX"
	"gnu_cxx 64 dyn"
	MUTATEEgnucxx64dynCXX)

IF (MUTATEEgnucxx64dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/arm64-unknown-linux/srclists.cmake)
if(TARGET symtabAPI)
add_executable (symtab_group_test.dyn_g++_64_pic_none ${SOURCE_LIST_11})
foreach (f ${SOURCE_LIST_11})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(symtab_group_test.dyn_g++_64_pic_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (symtab_group_test.dyn_g++_64_pic_none testA)


INSTALL (TARGETS
		symtab_group_test.dyn_g++_64_pic_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
