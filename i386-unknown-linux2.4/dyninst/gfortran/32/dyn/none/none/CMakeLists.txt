# CMakeLists for dyninst/gfortran/32/dyn/none/none
set (CMAKE_Fortran_FLAGS "-g ${MUTATOR_DEFINES}  -g ${MUTATEE_G77_FFLAGS}   ")
set (CMAKE_Fortran_FLAGS_DEBUG "")
set (CMAKE_Fortran_FLAGS_RELEASE "")
set (CMAKE_Fortran_COMPILER "${M_gnu_fc}")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src")
include_directories("${PROJECT_SOURCE_DIR}/testsuite/src/dyninst")
add_definitions(-DSOLO_MUTATEE)
set (CMAKE_EXE_LINKER_FLAGS "${MUTATEE_G77_LDFLAGS}  ")
IF (NOT ${M_gnu_fc} MATCHES "NOTFOUND")
CHECK_MUTATEE_COMPILER ("${M_gnu_fc}"
	"-g ${MUTATOR_DEFINES}  -g ${MUTATEE_G77_FFLAGS}   "
	"${MUTATEE_G77_LDFLAGS}  "
	"Fortran"
	"gnu_fc 32 dyn"
	MUTATEEgnufc32dynFortran)

IF (MUTATEEgnufc32dynFortran MATCHES "1")
include (${PROJECT_SOURCE_DIR}/i386-unknown-linux2.4/srclists.cmake)
add_executable (test1_10F.dyn_gfortran_32_none_none ${SOURCE_LIST_20})
set_source_files_properties(${SOURCE_LIST_20} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_10F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_11F.dyn_gfortran_32_none_none ${SOURCE_LIST_12})
set_source_files_properties(${SOURCE_LIST_12} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_11F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_12F.dyn_gfortran_32_none_none ${SOURCE_LIST_41})
set_source_files_properties(${SOURCE_LIST_41} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_12F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_13F.dyn_gfortran_32_none_none ${SOURCE_LIST_81})
set_source_files_properties(${SOURCE_LIST_81} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_13F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_14F.dyn_gfortran_32_none_none ${SOURCE_LIST_100})
set_source_files_properties(${SOURCE_LIST_100} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_14F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_16F.dyn_gfortran_32_none_none ${SOURCE_LIST_23})
set_source_files_properties(${SOURCE_LIST_23} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_16F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_17F.dyn_gfortran_32_none_none ${SOURCE_LIST_35})
set_source_files_properties(${SOURCE_LIST_35} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_17F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_18F.dyn_gfortran_32_none_none ${SOURCE_LIST_77})
set_source_files_properties(${SOURCE_LIST_77} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_18F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_19F.dyn_gfortran_32_none_none ${SOURCE_LIST_89})
set_source_files_properties(${SOURCE_LIST_89} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_19F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_1F.dyn_gfortran_32_none_none ${SOURCE_LIST_57})
set_source_files_properties(${SOURCE_LIST_57} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_1F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_20F.dyn_gfortran_32_none_none ${SOURCE_LIST_52})
set_source_files_properties(${SOURCE_LIST_52} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_20F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_25F.dyn_gfortran_32_none_none ${SOURCE_LIST_13})
set_source_files_properties(${SOURCE_LIST_13} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_25F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_29F.dyn_gfortran_32_none_none ${SOURCE_LIST_45})
set_source_files_properties(${SOURCE_LIST_45} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_29F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_2F.dyn_gfortran_32_none_none ${SOURCE_LIST_38})
set_source_files_properties(${SOURCE_LIST_38} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_2F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_31F.dyn_gfortran_32_none_none ${SOURCE_LIST_63})
set_source_files_properties(${SOURCE_LIST_63} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_31F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_32F.dyn_gfortran_32_none_none ${SOURCE_LIST_10})
set_source_files_properties(${SOURCE_LIST_10} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_32F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_34F.dyn_gfortran_32_none_none ${SOURCE_LIST_53})
set_source_files_properties(${SOURCE_LIST_53} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_34F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_36F.dyn_gfortran_32_none_none ${SOURCE_LIST_30})
set_source_files_properties(${SOURCE_LIST_30} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_36F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_3F.dyn_gfortran_32_none_none ${SOURCE_LIST_73})
set_source_files_properties(${SOURCE_LIST_73} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_3F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_4F.dyn_gfortran_32_none_none ${SOURCE_LIST_103})
set_source_files_properties(${SOURCE_LIST_103} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_4F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_5F.dyn_gfortran_32_none_none ${SOURCE_LIST_55})
set_source_files_properties(${SOURCE_LIST_55} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_5F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_6F.dyn_gfortran_32_none_none ${SOURCE_LIST_65})
set_source_files_properties(${SOURCE_LIST_65} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_6F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_7F.dyn_gfortran_32_none_none ${SOURCE_LIST_2})
set_source_files_properties(${SOURCE_LIST_2} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_7F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_8F.dyn_gfortran_32_none_none ${SOURCE_LIST_101})
set_source_files_properties(${SOURCE_LIST_101} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_8F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)
add_executable (test1_9F.dyn_gfortran_32_none_none ${SOURCE_LIST_49})
set_source_files_properties(${SOURCE_LIST_49} PROPERTIES LANGUAGE Fortran)
set_target_properties(test1_9F.dyn_gfortran_32_none_none PROPERTIES LINKER_LANGUAGE Fortran)


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
