cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

# There is a bug in 3.19.0 that causes .S files to be treated like C files
if(CMAKE_VERSION VERSION_EQUAL "3.19.0")
    message(FATAL_ERROR "Dyninst cannot use CMake version 3.19.0")
endif()

set(DYNINST_MAJOR_VERSION 12)
set(DYNINST_MINOR_VERSION 2)
set(DYNINST_PATCH_VERSION 1)

set(DYNINST_SOVERSION "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}")
set(DYNINST_LIBVERSION "${DYNINST_SOVERSION}.${DYNINST_PATCH_VERSION}")
set(DYNINST_VERSION "${DYNINST_LIBVERSION}")

project(
    Dyninst
    VERSION ${DYNINST_VERSION}
    DESCRIPTION "Tools for binary instrumentation, analysis, and modification"
    HOMEPAGE_URL "https://github.com/dyninst/dyninst"
    LANGUAGES C CXX ASM)

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake"
     "${PROJECT_SOURCE_DIR}/cmake/tpls" "${PROJECT_SOURCE_DIR}/cmake/Modules")

# These need to be done before anything else
include(DyninstLibrarySettings)
include(DyninstOptions)

# Set up Dyninst internals
include(DyninstPlatform)
include(DyninstCapArchDef)
include(DyninstLanguageStandards)
include(DyninstWarnings)
include(DyninstOptimization)

# Locate third-party libraries
find_package(Threads)
include(DyninstBoost)
include(DyninstTBB)
include(DyninstElfUtils)
include(DyninstLibIberty)
include(DyninstThread_DB)
include(DyninstValgrind)

if(USE_OpenMP)
    find_package(OpenMP REQUIRED)
else()
	# Dummy target so we don't have to check 'USE_OpenMP' everywhere
	add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
endif()

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_BINARY_DIR}/common/h)
set(HEADER_DIRS
    common
    dataflowAPI
    dyninstAPI
    instructionAPI
    parseAPI
    patchAPI
    proccontrol
    stackwalk
    symtabAPI)
if(NOT DYNINST_OS_Windows)
    set(HEADER_DIRS ${HEADER_DIRS} dwarf elf symlite)
endif()

foreach(dir ${HEADER_DIRS})
    include_directories(${PROJECT_SOURCE_DIR}/${dir}/h)
endforeach()

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/external)

# Component time
add_subdirectory(common)
add_subdirectory(elf)

if(NOT DYNINST_OS_Windows)
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
    add_subdirectory(parseThat)
endif()

add_subdirectory(dyninstAPI_RT)

include(DyninstInstall)
