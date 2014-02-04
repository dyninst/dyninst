# CMakeLists for proccontrol/g++/64/dyn/none/none
set (CMAKE_CXX_FLAGS "-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    ")
set (CMAKE_CXX_FLAGS_DEBUG "")
set (CMAKE_CXX_FLAGS_RELEASE "")
set (CMAKE_CXX_COMPILER "${M_gnu_cxx}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/proccontrol")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic")
IF (NOT ${M_gnu_cxx} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_cxx}"
	"-g ${MUTATOR_DEFINES}  ${CXXFLAGS} ${MUTATEE_CXXFLAGS_GNU}    "
	"${MUTATEE_LDFLAGS_GNU}  -Wl,-export-dynamic"
	"CXX"
	"gnu_cxx 64 dyn"
	MUTATEEgnucxx64dynCXX)

IF (MUTATEEgnucxx64dynCXX MATCHES "1")
include (${PROJECT_SOURCE_DIR}/amd64-unknown-freebsd7.2/srclists.cmake)
add_executable (pc_addlibrary.dyn_g++_64_none_none ${SOURCE_LIST_48})
set_source_files_properties(${SOURCE_LIST_48} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_addlibrary.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_addlibrary.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_breakpoint.dyn_g++_64_none_none ${SOURCE_LIST_37})
set_source_files_properties(${SOURCE_LIST_37} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_breakpoint.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_breakpoint.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_detach.dyn_g++_64_none_none ${SOURCE_LIST_15})
set_source_files_properties(${SOURCE_LIST_15} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_detach.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_detach.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_groups.dyn_g++_64_none_none ${SOURCE_LIST_47})
set_source_files_properties(${SOURCE_LIST_47} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_groups.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_groups.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_irpc.dyn_g++_64_none_none ${SOURCE_LIST_73})
set_source_files_properties(${SOURCE_LIST_73} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_irpc.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_irpc.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_launch.dyn_g++_64_none_none ${SOURCE_LIST_74})
set_source_files_properties(${SOURCE_LIST_74} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_launch.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_launch.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_library.dyn_g++_64_none_none ${SOURCE_LIST_32})
set_source_files_properties(${SOURCE_LIST_32} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_library.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_library.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_singlestep.dyn_g++_64_none_none ${SOURCE_LIST_34})
set_source_files_properties(${SOURCE_LIST_34} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_singlestep.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_singlestep.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_stat.dyn_g++_64_none_none ${SOURCE_LIST_13})
set_source_files_properties(${SOURCE_LIST_13} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_stat.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_stat.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_temp_detach.dyn_g++_64_none_none ${SOURCE_LIST_5})
set_source_files_properties(${SOURCE_LIST_5} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_temp_detach.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_temp_detach.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate.dyn_g++_64_none_none ${SOURCE_LIST_30})
set_source_files_properties(${SOURCE_LIST_30} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_terminate.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_terminate.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_terminate_stopped.dyn_g++_64_none_none ${SOURCE_LIST_56})
set_source_files_properties(${SOURCE_LIST_56} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_terminate_stopped.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_terminate_stopped.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread.dyn_g++_64_none_none ${SOURCE_LIST_44})
set_source_files_properties(${SOURCE_LIST_44} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_thread.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_thread.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})
add_executable (pc_thread_cont.dyn_g++_64_none_none ${SOURCE_LIST_24})
set_source_files_properties(${SOURCE_LIST_24} PROPERTIES LANGUAGE CXX)
set_target_properties(pc_thread_cont.dyn_g++_64_none_none PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (pc_thread_cont.dyn_g++_64_none_none ${CMAKE_THREAD_LIBS_INIT})


INSTALL (TARGETS
		pc_addlibrary.dyn_g++_64_none_none
		pc_breakpoint.dyn_g++_64_none_none
		pc_detach.dyn_g++_64_none_none
		pc_groups.dyn_g++_64_none_none
		pc_irpc.dyn_g++_64_none_none
		pc_launch.dyn_g++_64_none_none
		pc_library.dyn_g++_64_none_none
		pc_singlestep.dyn_g++_64_none_none
		pc_stat.dyn_g++_64_none_none
		pc_temp_detach.dyn_g++_64_none_none
		pc_terminate.dyn_g++_64_none_none
		pc_terminate_stopped.dyn_g++_64_none_none
		pc_thread.dyn_g++_64_none_none
		pc_thread_cont.dyn_g++_64_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_cxx failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_cxx not present")
ENDIF()
