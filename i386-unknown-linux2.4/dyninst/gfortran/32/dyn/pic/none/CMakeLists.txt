# CMakeLists for dyninst/gfortran/32/dyn/pic/none
set (CMAKE_C_FLAGS "-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  -g ${MUTATEE_G77_FFLAGS}   -fPIC")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_fc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_G77_LDFLAGS}  ")
IF (NOT ${M_gnu_fc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_fc}"
	"-g -I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  -g ${MUTATEE_G77_FFLAGS}   -fPIC"
	"${MUTATEE_G77_LDFLAGS}  "
	"fortran"
	"gnu_fc 32 dyn"
	MUTATEEgnufc32dynfortran)

IF (MUTATEEgnufc32dynfortran MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (test1_10Fdyn_gfortran_32_pic_none ${SOURCE_LIST_20})
add_executable (test1_11Fdyn_gfortran_32_pic_none ${SOURCE_LIST_12})
add_executable (test1_12Fdyn_gfortran_32_pic_none ${SOURCE_LIST_41})
add_executable (test1_13Fdyn_gfortran_32_pic_none ${SOURCE_LIST_81})
add_executable (test1_14Fdyn_gfortran_32_pic_none ${SOURCE_LIST_100})
add_executable (test1_16Fdyn_gfortran_32_pic_none ${SOURCE_LIST_23})
add_executable (test1_17Fdyn_gfortran_32_pic_none ${SOURCE_LIST_35})
add_executable (test1_18Fdyn_gfortran_32_pic_none ${SOURCE_LIST_77})
add_executable (test1_19Fdyn_gfortran_32_pic_none ${SOURCE_LIST_89})
add_executable (test1_1Fdyn_gfortran_32_pic_none ${SOURCE_LIST_57})
add_executable (test1_20Fdyn_gfortran_32_pic_none ${SOURCE_LIST_52})
add_executable (test1_25Fdyn_gfortran_32_pic_none ${SOURCE_LIST_13})
add_executable (test1_29Fdyn_gfortran_32_pic_none ${SOURCE_LIST_45})
add_executable (test1_2Fdyn_gfortran_32_pic_none ${SOURCE_LIST_38})
add_executable (test1_31Fdyn_gfortran_32_pic_none ${SOURCE_LIST_63})
add_executable (test1_32Fdyn_gfortran_32_pic_none ${SOURCE_LIST_10})
add_executable (test1_34Fdyn_gfortran_32_pic_none ${SOURCE_LIST_53})
add_executable (test1_36Fdyn_gfortran_32_pic_none ${SOURCE_LIST_30})
add_executable (test1_3Fdyn_gfortran_32_pic_none ${SOURCE_LIST_73})
add_executable (test1_4Fdyn_gfortran_32_pic_none ${SOURCE_LIST_103})
add_executable (test1_5Fdyn_gfortran_32_pic_none ${SOURCE_LIST_55})
add_executable (test1_6Fdyn_gfortran_32_pic_none ${SOURCE_LIST_65})
add_executable (test1_7Fdyn_gfortran_32_pic_none ${SOURCE_LIST_2})
add_executable (test1_8Fdyn_gfortran_32_pic_none ${SOURCE_LIST_101})
add_executable (test1_9Fdyn_gfortran_32_pic_none ${SOURCE_LIST_49})


INSTALL (TARGETS
		test1_10Fdyn_gfortran_32_pic_none
		test1_11Fdyn_gfortran_32_pic_none
		test1_12Fdyn_gfortran_32_pic_none
		test1_13Fdyn_gfortran_32_pic_none
		test1_14Fdyn_gfortran_32_pic_none
		test1_16Fdyn_gfortran_32_pic_none
		test1_17Fdyn_gfortran_32_pic_none
		test1_18Fdyn_gfortran_32_pic_none
		test1_19Fdyn_gfortran_32_pic_none
		test1_1Fdyn_gfortran_32_pic_none
		test1_20Fdyn_gfortran_32_pic_none
		test1_25Fdyn_gfortran_32_pic_none
		test1_29Fdyn_gfortran_32_pic_none
		test1_2Fdyn_gfortran_32_pic_none
		test1_31Fdyn_gfortran_32_pic_none
		test1_32Fdyn_gfortran_32_pic_none
		test1_34Fdyn_gfortran_32_pic_none
		test1_36Fdyn_gfortran_32_pic_none
		test1_3Fdyn_gfortran_32_pic_none
		test1_4Fdyn_gfortran_32_pic_none
		test1_5Fdyn_gfortran_32_pic_none
		test1_6Fdyn_gfortran_32_pic_none
		test1_7Fdyn_gfortran_32_pic_none
		test1_8Fdyn_gfortran_32_pic_none
		test1_9Fdyn_gfortran_32_pic_none
	DESTINATION ${INSTALL_DIR})

ELSE()
MESSAGE (STATUS "Compiler check for gnu_fc failed")
ENDIF()
ELSE()
MESSAGE (STATUS "Compiler gnu_fc not present")
ENDIF()
