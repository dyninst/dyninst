# CMakeLists for i386-unknown-linux2.4/dyninst/gfortran/32/dyn/none/none
set (CMAKE_C_FLAGS "-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/dyninst ${MUTATOR_DEFINES}  -g ${MUTATEE_G77_FFLAGS}    -g")
set (CMAKE_C_FLAGS_DEBUG "")
set (CMAKE_C_FLAGS_RELEASE "")
set (CMAKE_C_COMPILER "${M_gnu_fc}")
set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_G77_LDFLAGS}  ")
include (../../../../../../srclists.cmake)
add_executable (test1_10F.dyn_gfortran_32_none_none ${SOURCE_LIST_20})
add_executable (test1_11F.dyn_gfortran_32_none_none ${SOURCE_LIST_12})
add_executable (test1_12F.dyn_gfortran_32_none_none ${SOURCE_LIST_40})
add_executable (test1_13F.dyn_gfortran_32_none_none ${SOURCE_LIST_82})
add_executable (test1_14F.dyn_gfortran_32_none_none ${SOURCE_LIST_99})
add_executable (test1_16F.dyn_gfortran_32_none_none ${SOURCE_LIST_23})
add_executable (test1_17F.dyn_gfortran_32_none_none ${SOURCE_LIST_34})
add_executable (test1_18F.dyn_gfortran_32_none_none ${SOURCE_LIST_78})
add_executable (test1_19F.dyn_gfortran_32_none_none ${SOURCE_LIST_90})
add_executable (test1_1F.dyn_gfortran_32_none_none ${SOURCE_LIST_58})
add_executable (test1_20F.dyn_gfortran_32_none_none ${SOURCE_LIST_52})
add_executable (test1_25F.dyn_gfortran_32_none_none ${SOURCE_LIST_13})
add_executable (test1_29F.dyn_gfortran_32_none_none ${SOURCE_LIST_44})
add_executable (test1_2F.dyn_gfortran_32_none_none ${SOURCE_LIST_37})
add_executable (test1_31F.dyn_gfortran_32_none_none ${SOURCE_LIST_65})
add_executable (test1_32F.dyn_gfortran_32_none_none ${SOURCE_LIST_10})
add_executable (test1_34F.dyn_gfortran_32_none_none ${SOURCE_LIST_53})
add_executable (test1_36F.dyn_gfortran_32_none_none ${SOURCE_LIST_29})
add_executable (test1_3F.dyn_gfortran_32_none_none ${SOURCE_LIST_74})
add_executable (test1_4F.dyn_gfortran_32_none_none ${SOURCE_LIST_102})
add_executable (test1_5F.dyn_gfortran_32_none_none ${SOURCE_LIST_55})
add_executable (test1_6F.dyn_gfortran_32_none_none ${SOURCE_LIST_67})
add_executable (test1_7F.dyn_gfortran_32_none_none ${SOURCE_LIST_2})
add_executable (test1_8F.dyn_gfortran_32_none_none ${SOURCE_LIST_100})
add_executable (test1_9F.dyn_gfortran_32_none_none ${SOURCE_LIST_48})


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
	DESTINATION testsuite)
