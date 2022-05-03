cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND CMAKE_CURRENT_SOURCE_DIR STREQUAL
                                                  CMAKE_SOURCE_DIR)
    message(STATUS "Warning! Building from the source directory is not recommended.")
    message(
        STATUS
            "If this is not intentional, please remove 'CMakeCache.txt' and 'CMakeFiles' and build from a separate directory"
        )
    message(AUTHOR_WARNING "In-source build")
endif()

# There is a bug in 3.19.0 that causes .S files to be treated like C files
if(CMAKE_VERSION VERSION_EQUAL "3.19.0")
    message(FATAL_ERROR "Dyninst cannot use CMake version 3.19.0")
endif()

set(DYNINST_MAJOR_VERSION 12)
set(DYNINST_MINOR_VERSION 2)
set(DYNINST_PATCH_VERSION 0)
set(DYNINST_VERSION
    ${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}.${DYNINST_PATCH_VERSION})

project(
    Dyninst
    VERSION ${DYNINST_VERSION}
    DESCRIPTION "Tools for binary instrumentation, analysis, and modification"
    HOMEPAGE_URL "https://github.com/dyninst/dyninst"
    LANGUAGES C CXX)

set(DYNINST_ROOT ${PROJECT_SOURCE_DIR})
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_MESSAGE LAZY) # only report new installations

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake"
     "${PROJECT_SOURCE_DIR}/cmake/tpls" "${PROJECT_SOURCE_DIR}/cmake/Modules")

include(DyninstUtilities)

dyninst_add_option(
    STERILE_BUILD "Do not download/build any third-party dependencies from source" OFF
    ADVANCED)

# Set the C and C++ language standards
include(DyninstLanguageStandards)
include(DyninstOptions)
include(DyninstInstallDirs)

# Find the necessary third-party libraries
find_package(Thread_DB)
find_package(Threads REQUIRED)

include(DyninstBoost)
include(DyninstTBB)
include(DyninstElfUtils)
include(DyninstLibIberty)
include(DyninstLibrary)

if(USE_OpenMP)
    find_package(OpenMP REQUIRED)
endif()

configure_file(cmake/version.h.in ${PROJECT_BINARY_DIR}/common/h/dyninstversion.h @ONLY)

# global include interface library
dyninst_add_interface_library(headers "Interface target providing include paths only")
target_include_directories(
    headers
    INTERFACE $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
              $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
              $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/common/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dataflowAPI/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dyninstAPI/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/instructionAPI/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/parseAPI/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/patchAPI/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/proccontrol/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/stackwalk/h>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/symtabAPI/h>)
if(NOT ${PLATFORM} MATCHES nt)
    target_include_directories(
        headers
        INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dwarf/h>
                  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/elf/h>
                  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/symlite/h>)
endif()

dyninst_add_option(ADD_VALGRIND_ANNOTATIONS "Enable annotations for Valgrind analysis"
                   OFF)
if(ADD_VALGRIND_ANNOTATIONS)
    find_package(Valgrind REQUIRED)
    include_directories(${Valgrind_INCLUDE_DIRS})
    add_compile_definitions(ENABLE_VG_ANNOTATIONS)
endif()

target_include_directories(
    headers
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external>
              $<BUILD_INTERFACE:${TBB_INCLUDE_DIRS}>)

# Component time
add_subdirectory(common)
if(NOT ${PLATFORM} MATCHES nt)
    add_subdirectory(elf)
    add_subdirectory(dwarf)
    add_subdirectory(symlite)
endif()
add_subdirectory(instructionAPI)
add_subdirectory(symtabAPI)
add_subdirectory(parseAPI)
add_subdirectory(proccontrol)
add_subdirectory(stackwalk)
add_subdirectory(patchAPI)

if(${SYMREADER} MATCHES symtabAPI)
    add_subdirectory(dyninstAPI)
    add_subdirectory(dynC_API)
endif()

if(NOT ${PLATFORM} MATCHES nt AND BUILD_PARSE_THAT)
    add_subdirectory(parseThat)
endif()

add_subdirectory(dyninstAPI_RT)
set(VERSION_STRING
    "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}.${DYNINST_PATCH_VERSION}")
set(DYNINST_NAME "DyninstAPI-${VERSION_STRING}")

# add_subdirectory(testsuite)

# Copy out all of the cmake files so they can be used by the Testsuite
install(DIRECTORY ${DYNINST_ROOT}/cmake/ DESTINATION ${INSTALL_CMAKE_DIR})
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    install(FILES ${PROJECT_BINARY_DIR}/CMakeCache.txt DESTINATION ${INSTALL_CMAKE_DIR})
endif()

include(DyninstInstall)

dyninst_print_features()
