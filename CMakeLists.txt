cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

# There is a bug in 3.19.0 that causes .S files to be treated like C files
if(CMAKE_VERSION VERSION_EQUAL "3.19.0")
  message(FATAL_ERROR "Dyninst cannot use CMake version 3.19.0")
endif()

# find_package() uses upper-case <PACKAGENAME>_ROOT variables.
if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

set(DYNINST_MAJOR_VERSION 13)
set(DYNINST_MINOR_VERSION 0)
set(DYNINST_PATCH_VERSION 0)

set(DYNINST_SOVERSION "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}")
set(DYNINST_VERSION "${DYNINST_SOVERSION}.${DYNINST_PATCH_VERSION}")

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
include(DyninstThreads)
include(DyninstBoost)
include(DyninstTBB)
include(DyninstElfUtils)
include(DyninstLibIberty)
include(DyninstThread_DB)
include(DyninstValgrind)
include(DyninstOpenMP)

add_subdirectory(common)
add_subdirectory(elf)
add_subdirectory(dwarf)
add_subdirectory(symlite)
add_subdirectory(instructionAPI)
add_subdirectory(symtabAPI)
add_subdirectory(parseAPI)
add_subdirectory(proccontrol)
add_subdirectory(stackwalk)
add_subdirectory(patchAPI)
add_subdirectory(dyninstAPI)
add_subdirectory(dynC_API)
add_subdirectory(parseThat)
add_subdirectory(dyninstAPI_RT)

include(DyninstInstall)
