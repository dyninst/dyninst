# CMakeLists for dyninst/gfortran/32/dyn/none/none
set (CMAKE_C_FLAGS "-g ${MUTATOR_DEFINES}  -g ${MUTATEE_G77_FFLAGS}   ")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_fc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
message( STATUS "Old linker flags were ${CMAKE_EXE_LINKER_FLAGS}, new are ${MUTATEE_G77_LDFLAGS}  ")
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_G77_LDFLAGS}  ")
IF (NOT ${M_gnu_fc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_fc}"
	"-g ${MUTATOR_DEFINES}  -g ${MUTATEE_G77_FFLAGS}   "
	"${MUTATEE_G77_LDFLAGS}  "
	"fortran"
	"gnu_fc 32 dyn"
	MUTATEEgnufc32dynfortran)

IF (MUTATEEgnufc32dynfortran MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (test1_10F.dyn_gfortran_32_none_none ${SOURCE_LIST_20})
add_executable (test1_11F.dyn_gfortran_32_none_none ${SOURCE_LIST_12})
add_executable (test1_12F.dyn_gfortran_32_none_none ${SOURCE_LIST_41})
add_executable (test1_13F.dyn_gfortran_32_none_none ${SOURCE_LIST_81})
add_executable (test1_14F.dyn_gfortran_32_none_none ${SOURCE_LIST_100})
add_executable (test1_16F.dyn_gfortran_32_none_none ${SOURCE_LIST_23})
add_executable (test1_17F.dyn_gfortran_32_none_none ${SOURCE_LIST_35})
add_executable (test1_18F.dyn_gfortran_32_none_none ${SOURCE_LIST_77})
add_executable (test1_19F.dyn_gfortran_32_none_none ${SOURCE_LIST_89})
add_executable (test1_1F.dyn_gfortran_32_none_none ${SOURCE_LIST_57})
add_executable (test1_20F.dyn_gfortran_32_none_none ${SOURCE_LIST_52})
add_executable (test1_25F.dyn_gfortran_32_none_none ${SOURCE_LIST_13})
add_executable (test1_29F.dyn_gfortran_32_none_none ${SOURCE_LIST_45})
add_executable (test1_2F.dyn_gfortran_32_none_none ${SOURCE_LIST_38})
add_executable (test1_31F.dyn_gfortran_32_none_none ${SOURCE_LIST_63})
add_executable (test1_32F.dyn_gfortran_32_none_none ${SOURCE_LIST_10})
add_executable (test1_34F.dyn_gfortran_32_none_none ${SOURCE_LIST_53})
add_executable (test1_36F.dyn_gfortran_32_none_none ${SOURCE_LIST_30})
add_executable (test1_3F.dyn_gfortran_32_none_none ${SOURCE_LIST_73})
add_executable (test1_4F.dyn_gfortran_32_none_none ${SOURCE_LIST_103})
add_executable (test1_5F.dyn_gfortran_32_none_none ${SOURCE_LIST_55})
add_executable (test1_6F.dyn_gfortran_32_none_none ${SOURCE_LIST_65})
add_executable (test1_7F.dyn_gfortran_32_none_none ${SOURCE_LIST_2})
add_executable (test1_8F.dyn_gfortran_32_none_none ${SOURCE_LIST_101})
add_executable (test1_9F.dyn_gfortran_32_none_none ${SOURCE_LIST_49})


INSTALL (TARGETS
		test1_10F.dyn_gfortran_32_none_none
		test1_11F.dyn_gfortran_32_none_none
		test1_12F.dyn_gfortran_32_none_none
		test1_13F.dyn_gfortran_32_none_none
		test1_14F.dyn_gfortran_32_none_none
		test1_16F.dyn_gfortran_32_none_none
		test1_17F.dyn_gfortran_32_none_none
		test1_18F.dyn_gfortran_32_none_none
		test1_19F.dyn_gfortran_32_none_none
		test1_1F.dyn_gfortran_32_none_none
		test1_20F.dyn_gfortran_32_none_none
		test1_25F.dyn_gfortran_32_none_none
		test1_29F.dyn_gfortran_32_none_none
		test1_2F.dyn_gfortran_32_none_none
		test1_31F.dyn_gfortran_32_none_none
		test1_32F.dyn_gfortran_32_none_none
		test1_34F.dyn_gfortran_32_none_none
		test1_36F.dyn_gfortran_32_none_none
		test1_3F.dyn_gfortran_32_none_none
		test1_4F.dyn_gfortran_32_none_none
		test1_5F.dyn_gfortran_32_none_none
		test1_6F.dyn_gfortran_32_none_none
		test1_7F.dyn_gfortran_32_none_none
		test1_8F.dyn_gfortran_32_none_none
		test1_9F.dyn_gfortran_32_none_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_fc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_fc not present")
ENDIF()
