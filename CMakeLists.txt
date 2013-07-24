cmake_minimum_required (VERSION 2.6)
project (Dyninst-TestSuite)

include (c++11.cmake)

find_package (Dyninst REQUIRED CONFIG)
find_package (Threads)
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
#TODO BG/Q
elseif(WIN32)
FIND_PROGRAM (M_native_cc NAMES cl)
FIND_PROGRAM (M_native_cxx NAMES cl)
FIND_PROGRAM (M_native_linker NAMES link)
set (MUTATEE_VC32dyn 1)
set (MUTATEE_VCxx32dyn 1)
endif (UNIX)

include (checkMutateeCompiler.cmake)
include (${DYNINST_PLATFORM}/cmake-compilers.txt)

add_library (testA SHARED src/libtestA.c)
add_library (testB SHARED src/libtestB.c)
if(UNIX)
add_library (Test12 SHARED src/dyninst/libTest12.c)
install (TARGETS Test12 
	LIBRARY DESTINATION ${INSTALL_DIR}
	RUNTIME DESTINATION ${INSTALL_DIR}
	ARCHIVE DESTINATION ${INSTALL_DIR}
)
endif()
install (TARGETS testA testB 
	LIBRARY DESTINATION ${INSTALL_DIR}
	RUNTIME DESTINATION ${INSTALL_DIR}
	ARCHIVE DESTINATION ${INSTALL_DIR}
)

if (${MUTATEE_gcc64dyn} AND ${MUTATEE_gcc32dyn})
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

