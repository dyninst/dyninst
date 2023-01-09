cmake_minimum_required(VERSION 3.4.0)

# There is a bug in 3.19.0 that causes .S files to be treated like C files
if(CMAKE_VERSION VERSION_EQUAL "3.19.0")
    message(FATAL_ERROR "Dyninst cannot use CMake version 3.19.0")
endif()

project(Dyninst)

set(DYNINST_ROOT ${PROJECT_SOURCE_DIR})
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(STERILE_BUILD
    ON
    CACHE BOOL "Do not download/build any third-party dependencies from source")

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
     isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

set(CMAKE_MODULE_PATH "${DYNINST_ROOT}/cmake" "${DYNINST_ROOT}/cmake/Modules"
                      ${CMAKE_MODULE_PATH})

# Set the C and C++ language standards
include(LanguageStandards)

# Find the necessary third-party libraries
find_package(ThreadDB)
find_package(Threads)
include(Boost)
include(ThreadingBuildingBlocks)
include(ElfUtils)

if(UNIX)
    include(LibIberty REQUIRED)
endif()

include(shared)

if(USE_OpenMP)
    find_package(OpenMP REQUIRED)
endif()

configure_file(cmake/version.h.in common/h/dyninstversion.h)
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
if(NOT ${PLATFORM} MATCHES nt)
    set(HEADER_DIRS ${HEADER_DIRS} dwarf elf)
endif()

foreach(dir ${HEADER_DIRS})
    include_directories(${DYNINST_ROOT}/${dir}/h)
endforeach()

set(ADD_VALGRIND_ANNOTATIONS
    OFF
    CACHE BOOL "Enable annotations for Valgrind analysis")
if(ADD_VALGRIND_ANNOTATIONS)
    find_package(Valgrind REQUIRED)
    include_directories(${Valgrind_INCLUDE_DIRS})
    add_definitions(-DENABLE_VG_ANNOTATIONS)
endif()

include_directories(${DYNINST_ROOT} ${DYNINST_ROOT}/external ${TBB_INCLUDE_DIRS})

# Component time
add_subdirectory(common)
if(NOT ${PLATFORM} MATCHES nt)
    add_subdirectory(elf)
    add_subdirectory(dwarf)
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

set(ENABLE_STATIC_LIBS
    NO
    CACHE STRING "Build static libraries as well?")

set(VERSION_STRING
    "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}.${DYNINST_PATCH_VERSION}")
set(DYNINST_NAME "DyninstAPI-${VERSION_STRING}")

if(TARGET boost)
    add_dependencies(common boost)
endif()

# add_subdirectory(testsuite)

# Copy out all of the cmake files so they can be used by the Testsuite
install(DIRECTORY ${DYNINST_ROOT}/cmake/ DESTINATION ${INSTALL_CMAKE_DIR})
install(FILES ${PROJECT_BINARY_DIR}/CMakeCache.txt DESTINATION ${INSTALL_CMAKE_DIR})
