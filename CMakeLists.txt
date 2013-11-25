cmake_minimum_required (VERSION 2.6)
project (Dyninst-TestSuite)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include (c++11.cmake)

#CMake tries to auto-add flags to link lines, which isn't helpful.  Blanking this variable should fix.
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

find_package (Dyninst REQUIRED CONFIG)
find_package (Threads)
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0"
			      "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0"
			      "1.53" "1.53.0" "1.54" "1.54.0")

find_package (Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

message (STATUS "Dyninst includes: ${DYNINST_INCLUDE_DIR}")
include_directories (${DYNINST_INCLUDE_DIR})
message (STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")
set (BUILD_SHARED_LIBS ON)

set (INSTALL_DIR "bin/testsuite" CACHE PATH "Testsuite installation directory")

# Build rules for the test libraries (libtestdyninst, libtestproccontrol, etc.)
# and the executables

if (UNIX)
enable_language(ASM-ATT)
elseif(WIN32)
enable_language(ASM_MASM)
add_definitions(-Dsnprintf=_snprintf)
endif(UNIX)

foreach (def ${DYNINST_INTERNAL_DEFINES})
  # add_definitions doesn't seem to get pulled in; that's okay, since
  # we don't want it anyway (we want to override for mutators/ees)
  add_definitions (${def}_test)
  set (MUTATEE_DEFINES "${MUTATEE_DEFINES} ${def}_test")
endforeach()

include_directories (src
                    src/dyninst
                    src/proccontrol
                    src/symtab
                    src/instruction)

set (LIBTESTSUITE_COMMON_SRCS
            src/test_lib.C 
            src/TestData.C
            src/TestMutator.C
            src/TestOutputDriver.C
            src/StdOutputDriver.C
            src/remotetest.C
            src/connection.C
            src/module.C
            src/test_lib_templates.C)

set (LIBTESTLAUNCH_SRCS
    src/CmdLine.C
    src/ParameterDict.C
    src/ResumeLog.C
    src/MutateeStart.C
    src/test_info_new.C
    ${DYNINST_PLATFORM}/test_info_new.gen.C)    

if(UNIX)
add_library(testSuite SHARED
    src/test_lib_soExecution.C
    ${LIBTESTSUITE_COMMON_SRCS})
add_library(testlaunch SHARED
    ${LIBTESTLAUNCH_SRCS})
target_link_libraries (testlaunch)
set(TESTSUITE_TESTLAUNCH_LINK testSuite testlaunch)
else()
add_library(testSuite SHARED
    src/test_lib_dllExecution.C
    ${LIBTESTSUITE_COMMON_SRCS}
    ${LIBTESTLAUNCH_SRCS})
set(TESTSUITE_TESTLAUNCH_LINK testSuite)
endif()

set_target_properties(${TESTSUITE_TESTLAUNCH_LINK} PROPERTIES COMPILE_DEFINITIONS TESTLIB_DLL_BUILD)

target_link_libraries (testSuite ${CMAKE_DL_LIBS})

add_library(DatabaseOutputDriver SHARED
				 src/DatabaseOutputDriver.C
				 )
target_link_libraries(DatabaseOutputDriver testSuite)
set_target_properties(DatabaseOutputDriver PROPERTIES PREFIX "")
if(WIN32)
	set_target_properties(DatabaseOutputDriver PROPERTIES COMPILE_DEFINITIONS os_windows_test)
	target_link_libraries(DatabaseOutputDriver ws2_32)
endif()

if(WIN32)
set (PLAT_SRC)
else()
set (PLAT_SRC src/dyninst/ParseThat.C
    src/dyninst/test_lib_test7.C
    src/dyninst/test_lib_test9.C)
endif()

add_library (testdyninst SHARED
            src/dyninst/dyninst_comp.C
            src/dyninst/test_lib_mutateeStart.C
            src/dyninst/Callbacks.C
            src/dyninst/Process_data.C
	    ${PLAT_SRC})
target_link_libraries (testdyninst testSuite dyninstAPI ${CMAKE_THREAD_LIBS_INIT})

add_library (testsymtab SHARED
            src/symtab/symtab_comp.C)
target_link_libraries (testsymtab testSuite symtabAPI ${CMAKE_THREAD_LIBS_INIT})

add_library (testinstruction SHARED
            src/instruction/instruction_comp.C)
target_link_libraries (testinstruction testSuite instructionAPI symtabAPI ${CMAKE_THREAD_LIBS_INIT})

add_library (testproccontrol SHARED
            src/proccontrol/proccontrol_comp.C)
target_link_libraries (testproccontrol testSuite pcontrol ${CMAKE_THREAD_LIBS_INIT})
set (COMPLIB_DEFS TESTLIB_DLL_BUILD COMPLIB_DLL_BUILD)
set (COMPLIB_TARGETS testdyninst testsymtab testinstruction testproccontrol)
set_target_properties(${COMPLIB_TARGETS} PROPERTIES COMPILE_DEFINITIONS "${COMPLIB_DEFS}")

if(WIN32)
set(RUNTESTS_UTILS src/runTests-utils-nt.C)
else()
set(RUNTESTS_UTILS src/runTests-utils.C)
endif()

add_executable (runTests 
               src/runTests.C
	       ${RUNTESTS_UTILS}
               src/test_driver_templates.C)
target_link_libraries (runTests)

if(WIN32)
set(TD_BE)
else()
set(TD_BE src/testdriver_be.C)
endif()
add_executable (test_driver
               src/test_driver.C
               src/test_driver_templates.C
               src/dynlmon.C
	       ${TD_BE}
               ${DYNINST_PLATFORM}/test_info_new.gen.C)
target_link_libraries (test_driver ${TESTSUITE_TESTLAUNCH_LINK})

if(WIN32)
else()
add_executable (testdriver_wrapper
               src/connection.C
               src/testdriver_wrapper.C)
install (TARGETS
        testdriver_wrapper
        DESTINATION ${INSTALL_DIR})
endif()

install (TARGETS
        test_driver
        runTests
	${TESTSUITE_TESTLAUNCH_LINK}
        testdyninst
        testsymtab
        testinstruction
        testproccontrol
	DatabaseOutputDriver
        DESTINATION ${INSTALL_DIR})


include (${DYNINST_PLATFORM}/cmake-mutators.txt)

ADD_CUSTOM_TARGET(test_names.txt
	COMMAND echo >> test_names.txt ${MUTATOR_NAME_LIST}
	VERBATIM)

if (UNIX)
# Compiler macros
FIND_PROGRAM(M_gnu_cc NAMES gcc)
MESSAGE(STATUS "Mutatee gcc: ${M_gnu_cc}")
FIND_PROGRAM(M_gnu_cxx NAMES g++)
MESSAGE(STATUS "Mutatee g++: ${M_gnu_cxx}")
FIND_PROGRAM(M_intel_cc NAMES icc)
MESSAGE(STATUS "Mutatee icc: ${M_intel_cc}")
FIND_PROGRAM(M_intel_CC NAMES icpc)
MESSAGE(STATUS "Mutatee icpc: ${M_intel_CC}")
FIND_PROGRAM(M_pg_cc NAMES pgcc)
MESSAGE(STATUS "Mutatee pgcc: ${M_pg_cc}")
FIND_PROGRAM(M_pg_cxx NAMES pgCC)
MESSAGE(STATUS "Mutatee pgCC: ${M_pg_cxx}")
FIND_PROGRAM(M_mpi_cc NAMES mpicc)
MESSAGE(STATUS "Mutatee mpicc: ${M_mpi_cc}")
FIND_PROGRAM(M_mpi_cxx NAMES mpicxx)
MESSAGE(STATUS "Mutatee mpicxx: ${M_mpi_cxx}")
FIND_PROGRAM(M_mpi_xlc NAMES mpixlc)
MESSAGE(STATUS "Mutatee mpixlc: ${M_mpi_xlc}")
FIND_PROGRAM(M_mpi_xlcxx NAMES mpixlcxx)
MESSAGE(STATUS "Mutatee mpixlcxx: ${M_mpi_xlcxx}")

elseif(WIN32)
FIND_PROGRAM (M_native_cc NAMES cl)
FIND_PROGRAM (M_native_cxx NAMES cl)
FIND_PROGRAM (M_native_linker NAMES link)
endif (UNIX)

include (checkMutateeCompiler.cmake)

add_library (testA SHARED src/libtestA.c)
add_library (testA_static STATIC src/libtestA.c)
add_library (testB SHARED src/libtestB.c)
add_library (testB_static STATIC src/libtestB.c)
set_target_properties(testA_static PROPERTIES OUTPUT_NAME testA)
set_target_properties(testB_static PROPERTIES OUTPUT_NAME testB)

if(UNIX)
add_library (Test12 SHARED src/dyninst/libTest12.c)
install (TARGETS Test12 
	LIBRARY DESTINATION ${INSTALL_DIR}
	RUNTIME DESTINATION ${INSTALL_DIR}
	ARCHIVE DESTINATION ${INSTALL_DIR}
)
endif()
install (TARGETS testA testB testA_static testB_static
	LIBRARY DESTINATION ${INSTALL_DIR}
	RUNTIME DESTINATION ${INSTALL_DIR}
	ARCHIVE DESTINATION ${INSTALL_DIR}
)

if(NOT ${PLATFORM} MATCHES nt)
add_library (testA_m32 SHARED src/libtestA.c)
add_library (testB_m32 SHARED src/libtestB.c)
add_library (Test12_m32 SHARED src/dyninst/libTest12.c)
install (TARGETS testA_m32 testB_m32 Test12_m32 LIBRARY DESTINATION ${INSTALL_DIR})

set_target_properties (testA_m32
                      PROPERTIES
                      COMPILE_FLAGS "-m32 -Dm32_test"
                      LINK_FLAGS "-m32")
set_target_properties (testB_m32
                      PROPERTIES
                      COMPILE_FLAGS "-m32 -Dm32_test"
                      LINK_FLAGS "-m32")
set_target_properties (Test12_m32
                      PROPERTIES
                      COMPILE_FLAGS "-m32 -Dm32_test"
                      LINK_FLAGS "-m32")
endif()

include (${DYNINST_PLATFORM}/cmake-mutatees.txt)

