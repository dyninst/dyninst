cmake_minimum_required (VERSION 2.6)
project (Dyninst-TestSuite)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include (c++11.cmake)

set (USE_CXX11_ABI "" CACHE STRING "Override the default GNU C++11 ABI setting")
if (NOT ("${USE_CXX11_ABI}" STREQUAL ""))
  if (${USE_CXX11_ABI})
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
  else()
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
  endif()
endif()


if (CMAKE_COMPILER_IS_GNUCXX  OR  ${CMAKE_C_COMPILER_ID} MATCHES Clang)
set (CMAKE_C_FLAGS_DEBUG "-O0 -g")
set (CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
else (MSVC)
set (CMAKE_C_FLAGS_DEBUG "/Od /Zi /MDd /D_DEBUG")
set (CMAKE_C_FLAGS_RELEASE "/O2 /MD /D NDEBUG")
set (CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /MDd /D_DEBUG")
set (CMAKE_CXX_FLAGS_RELEASE "/O2 /MD /D NDEBUG")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "/O2 /Zi /MD /D NDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /MD /D NDEBUG")
endif()


#CMake tries to auto-add flags to link lines, which isn't helpful.  Blanking this variable should fix.
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

find_package (Dyninst REQUIRED COMPONENTS common
  OPTIONAL_COMPONENTS symtabAPI dyninstAPI instructionAPI proccontrol)

find_package (Threads)
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0"
			      "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0"
			      "1.53" "1.53.0" "1.54" "1.54.0" "1.55" "1.55.0")

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

if (NOT ("${Boost_NO_BOOST_CMAKE}" STREQUAL "OFF"))
  message(STATUS "Disabling Boost's own CMake--known buggy in many cases")
  set(Boost_NO_BOOST_CMAKE ON)
endif()
if (NOT ("${PATH_BOOST}" STREQUAL ""))
  set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PATH_BOOST}/lib ${PATH_BOOST}/lib64)
  set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${PATH_BOOST}/include)
endif()

find_package (Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


message (STATUS "Dyninst includes: ${DYNINST_INCLUDE_DIR}")
include_directories (${DYNINST_INCLUDE_DIR})
message (STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")
set (BUILD_SHARED_LIBS ON)



if ("${DYNINST_PLATFORM}" MATCHES "bgq")
find_package( LaunchMON REQUIRED )
include_directories(${LAUNCHMON_INCLUDE_DIR})
add_definitions(${LAUNCHMON_DEFINITIONS})
endif()

set (INSTALL_DIR "bin/testsuite" CACHE PATH "Testsuite installation directory")

# Build rules for the test libraries (libtestdyninst, libtestproccontrol, etc.)
# and the executables

if (UNIX)
  enable_language(ASM-ATT)
  if ("${DYNINST_PLATFORM}" MATCHES "i386")
    enable_language(ASM_NASM)
  endif()
elseif(WIN32)
  enable_language(ASM_MASM)
  if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 19)
    add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
  else()
    add_definitions(-Dsnprintf=_snprintf)
  endif()
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

set (COMPLIB_DEFS TESTLIB_DLL_BUILD COMPLIB_DLL_BUILD)

if(TARGET dyninstAPI)
  add_library (testdyninst SHARED
    src/dyninst/dyninst_comp.C
    src/dyninst/test_lib_mutateeStart.C
    src/dyninst/Callbacks.C
    src/dyninst/Process_data.C
    ${PLAT_SRC})
  target_link_libraries (testdyninst testSuite dyninstAPI instructionAPI ${CMAKE_THREAD_LIBS_INIT})
  install(TARGETS testdyninst
    DESTINATION ${INSTALL_DIR})
  set_target_properties(testdyninst PROPERTIES COMPILE_DEFINITIONS "${COMPLIB_DEFS}")
endif()
if(TARGET symtabAPI)
  add_library (testsymtab SHARED
    src/symtab/symtab_comp.C)
  target_link_libraries (testsymtab testSuite symtabAPI common ${CMAKE_THREAD_LIBS_INIT})
  install(TARGETS testsymtab
    DESTINATION ${INSTALL_DIR})
  set_target_properties(testsymtab PROPERTIES COMPILE_DEFINITIONS "${COMPLIB_DEFS}")
endif()

if(TARGET instructionAPI)
  add_library (testinstruction SHARED
    src/instruction/instruction_comp.C)
  target_link_libraries (testinstruction testSuite instructionAPI symtabAPI common ${CMAKE_THREAD_LIBS_INIT})
  install(TARGETS testinstruction
    DESTINATION ${INSTALL_DIR})
  set_target_properties(testinstruction PROPERTIES COMPILE_DEFINITIONS "${COMPLIB_DEFS}")
endif()

if(TARGET pcontrol)
  add_library (testproccontrol SHARED
    src/proccontrol/proccontrol_comp.C)
  
  if(WIN32)
    target_link_libraries(testproccontrol testSuite pcontrol common ${CMAKE_THREAD_LIBS_INIT} ws2_32)
  else()
    target_link_libraries (testproccontrol testSuite pcontrol common ${CMAKE_THREAD_LIBS_INIT})
  endif()
  install(TARGETS testproccontrol
    DESTINATION ${INSTALL_DIR})
  set_target_properties(testproccontrol PROPERTIES COMPILE_DEFINITIONS "${COMPLIB_DEFS}")
endif()

if(WIN32)
set(RUNTESTS_UTILS src/runTests-utils-nt.C)
else()
set(RUNTESTS_UTILS src/runTests-utils.C)
endif()

if(WIN32)
set(TD_BE)
else()
set(TD_BE src/testdriver_be.C)
endif()

add_executable (runTests 
               src/runTests.C
	       ${RUNTESTS_UTILS}
               src/test_driver_templates.C)
target_link_libraries (runTests)

add_executable (test_driver
               src/test_driver.C
               src/test_driver_templates.C
               src/dynlmon.C
               ${TD_BE}
               ${DYNINST_PLATFORM}/test_info_new.gen.C)
target_link_libraries (test_driver ${TESTSUITE_TESTLAUNCH_LINK})
if (LAUNCHMON_FE_LIBRARIES)
target_link_libraries (test_driver ${LAUNCHMON_FE_LIBRARIES})
target_link_libraries (test_driver ${LAUNCHMON_BE_LIBRARIES})
endif()

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
	DatabaseOutputDriver
        DESTINATION ${INSTALL_DIR})


include (${DYNINST_PLATFORM}/cmake-mutators.txt)

FOREACH (m ${MUTATOR_NAME_LIST})
    file(APPEND ${CMAKE_BINARY_DIR}/test_names.txt "${m}\n")
ENDFOREACH()

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


#if (${MUTATEE_gcc64dyn} AND ${MUTATEE_gcc32dyn})
if (UNIX)
CHECK_MUTATEE_COMPILER(gcc "-m32" "-m32" C "" m32_works)
if(${m32_works})
message(STATUS "Building 32-bit helper libraries")
add_library (testA_m32 SHARED src/libtestA.c)
add_library (testB_m32 SHARED src/libtestB.c)
add_library (testA_m32_static STATIC src/libtestA.c)
add_library (testB_m32_static STATIC src/libtestB.c)
set_target_properties(testA_m32_static PROPERTIES OUTPUT_NAME testA_m32)
set_target_properties(testB_m32_static PROPERTIES OUTPUT_NAME testB_m32)
add_library (Test12_m32 SHARED src/dyninst/libTest12.c)

install (TARGETS testA_m32 testB_m32 Test12_m32 testA_m32_static testB_m32_static 
	LIBRARY DESTINATION ${INSTALL_DIR}
	ARCHIVE DESTINATION ${INSTALL_DIR})

set_target_properties (testA_m32 testA_m32_static
                      PROPERTIES
                      COMPILE_FLAGS "-m32 -Dm32_test"
                      LINK_FLAGS "-m32")
set_target_properties (testB_m32 testB_m32_static
                      PROPERTIES
                      COMPILE_FLAGS "-m32 -Dm32_test"
                      LINK_FLAGS "-m32")
set_target_properties (Test12_m32
                      PROPERTIES
                      COMPILE_FLAGS "-m32 -Dm32_test"
                      LINK_FLAGS "-m32")
endif()
endif()

include (${DYNINST_PLATFORM}/cmake-mutatees.txt)

