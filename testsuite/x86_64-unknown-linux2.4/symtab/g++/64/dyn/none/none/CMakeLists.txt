# CMakeLists for symtab/g++/64/dyn/none/none
set (CMAKE_CXX_FLAGS "${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}   -g -O0 ")
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
	"${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}   -g -O0 "
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"CXX"
	"gnu_cxx 64 dyn"
	MUTATEEgnucxx64dynCXX)

IF (MUTATEEgnucxx64dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/x86_64-unknown-linux2.4/srclists.cmake)
if(TARGET symtabAPI)
add_executable (symtab_group_test.dyn_g++_64_none_none ${SOURCE_LIST_35})
foreach (f ${SOURCE_LIST_35})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(symtab_group_test.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (symtab_group_test.dyn_g++_64_none_none testA)
add_executable (test_exception.dyn_g++_64_none_none ${SOURCE_LIST_71})
foreach (f ${SOURCE_LIST_71})
	get_source_file_property(lang ${f} LANGUAGE)
	if(lang MATCHES "C")
		set_source_files_properties(${f} PROPERTIES LANGUAGE CXX)
	endif()
	endforeach()
set_target_properties(test_exception.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)


INSTALL (TARGETS
		symtab_group_test.dyn_g++_64_none_none
		test_exception.dyn_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ENDIF()
ENDIF()
ENDIF()
