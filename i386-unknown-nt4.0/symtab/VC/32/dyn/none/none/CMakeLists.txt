# CMakeLists for symtab/VC/32/dyn/none/none
set (CMAKE_C_FLAGS "/D_DEBUG /I${PROJECT_SOURCE_DIR}/testsuite/src /I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  /TC ${CFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_native_cc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
message( STATUS "Old linker flags were "${CMAKE_EXE_LINKER_FLAGS}", new are ${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  ")
IF (NOT ${M_native_cc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_native_cc}"
	"/D_DEBUG /I${PROJECT_SOURCE_DIR}/testsuite/src /I${PROJECT_SOURCE_DIR}/testsuite/src/symtab ${MUTATOR_DEFINES}  /TC ${CFLAGS}   "
	"${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}  "
	"c"
	"native_cc 32 dyn"
	MUTATEEnativecc32dync)

IF (MUTATEEnativecc32dync MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-nt4.0/srclists.cmake)
add_executable (symtab_group_testdyn_VC_32_none_none ${SOURCE_LIST_29})
target_link_libraries (symtab_group_testdyn_VC_32_none_none testA_m32)


INSTALL (TARGETS
		symtab_group_testdyn_VC_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for native_cc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler native_cc not present")
ENDIF()
