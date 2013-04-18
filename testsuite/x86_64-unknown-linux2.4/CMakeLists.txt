# Build rules for the test libraries (libtestdyninst, libtestproccontrol, etc.)
# and the executables

if (UNIX)
enable_language(ASM-ATT)
endif(UNIX)

foreach (def ${UNIFIED_DEFINES})
  # add_definitions doesn't seem to get pulled in; that's okay, since
  # we don't want it anyway (we want to override for mutators/ees)
  remove_definitions (${def})
  add_definitions (${def}_test)
  set (MUTATEE_DEFINES "${MUTATEE_DEFINES} ${def}_test")
endforeach()

include_directories (../src
                    ../src/dyninst
                    ../src/proccontrol
                    ../src/symtab
                    ../src/instruction)

add_library (testSuite SHARED 
            ../src/test_lib.C 
            ../src/test_lib_soExecution.C
            ../src/TestData.C
            ../src/TestMutator.C
            ../src/TestOutputDriver.C
            ../src/StdOutputDriver.C
            ../src/remotetest.C
            ../src/connection.C
            ../src/module.C
            ../src/test_lib_templates.C)
target_link_libraries (testSuite dl)

add_library (testlaunch SHARED
            ../src/CmdLine.C
            ../src/ParameterDict.C
            ../src/ResumeLog.C
            ../src/MutateeStart.C
            ../src/test_info_new.C
            test_info_new.gen.C)
target_link_libraries (testlaunch)

add_library (testdyninst SHARED
            ../src/dyninst/dyninst_comp.C
            ../src/dyninst/test_lib_mutateeStart.C
            ../src/dyninst/Callbacks.C
            ../src/dyninst/Process_data.C
            ../src/dyninst/ParseThat.C
            ../src/dyninst/test_lib_test7.C
            ../src/dyninst/test_lib_test9.C)
target_link_libraries (testdyninst dyninstAPI pthread)

add_library (testsymtab SHARED
            ../src/symtab/symtab_comp.C)
target_link_libraries (testsymtab symtabAPI pthread)

add_library (testinstruction SHARED
            ../src/instruction/instruction_comp.C)
target_link_libraries (testinstruction instructionAPI pthread)

add_library (testproccontrol SHARED
            ../src/proccontrol/proccontrol_comp.C)
target_link_libraries (testproccontrol pcontrol pthread)

add_executable (runTests 
               ../src/runTests.C
               ../src/runTests-utils.C
               ../src/test_driver_templates.C)
target_link_libraries (runTests)

add_executable (test_driver
               ../src/test_driver.C
               ../src/test_driver_templates.C
               ../src/dynlmon.C
               ../src/testdriver_be.C
               test_info_new.gen.C)
target_link_libraries (test_driver testSuite testlaunch)


add_executable (testdriver_wrapper
               ../src/connection.C
               ../src/testdriver_wrapper.C)

install (TARGETS
        test_driver
        testdriver_wrapper
        runTests
        testSuite
        testlaunch
        testdyninst
        testsymtab
        testinstruction
        testproccontrol
        DESTINATION testsuite)



include (cmake-mutators.txt)



# Compiler macros
FIND_PROGRAM(M_gnu_cc NAMES gcc)
FIND_PROGRAM(M_gnu_cxx NAMES g++)
FIND_PROGRAM(M_intel_cc NAMES icc)
FIND_PROGRAM(M_intel_CC NAMES icpc)
FIND_PROGRAM(M_pg_cc NAMES pgcc)
FIND_PROGRAM(M_pg_cxx NAMES pgCC)

include (cmake-compilers.txt)

add_library (testA SHARED ../src/libtestA.c)
add_library (testB SHARED ../src/libtestB.c)
add_library (Test12 SHARED ../src/dyninst/libTest12.c)
install (TARGETS testA testB Test12 LIBRARY DESTINATION testsuite)

if (${MUTATEE_gcc32dyn})
add_library (testA_m32 SHARED ../src/libtestA.c)
add_library (testB_m32 SHARED ../src/libtestB.c)
add_library (Test12_m32 SHARED ../src/dyninst/libTest12.c)
install (TARGETS testA_m32 testB_m32 Test12_m32 LIBRARY DESTINATION testsuite)

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

include (cmake-mutatees.txt)

