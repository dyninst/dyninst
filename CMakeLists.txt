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

set(DYNINST_MAJOR_VERSION 11)
set(DYNINST_MINOR_VERSION 0)
set(DYNINST_PATCH_VERSION 1)
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

# for packaging
if(_DYNINST_BUILDING_TPLS)
    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/include/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_INCLUDE_DIR}
        COMPONENT "tpls"
        OPTIONAL)

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/bin/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_BIN_DIR}
        COMPONENT "tpls"
        OPTIONAL)

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/share/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_DOC_DIR}
        COMPONENT "tpls"
        OPTIONAL)

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/lib/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_LIB_DIR}
        COMPONENT "tpls"
        OPTIONAL FILES_MATCHING
        PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
        PATTERN "*${CMAKE_STATIC_LIBRARY_SUFFIX}")

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/lib/pkgconfig/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_LIB_DIR}/pkgconfig
        COMPONENT "tpls"
        OPTIONAL FILES_MATCHING
        PATTERN "*.pc")
endif()

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

if(BUILD_RTLIB)
    add_subdirectory(dyninstAPI_RT)
else()
    message(
        STATUS
            "Skipping DyninstAPI_RT. Be sure to build this library if you're using instrumentation."
        )
endif()
set(VERSION_STRING
    "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}.${DYNINST_PATCH_VERSION}")
set(DYNINST_NAME "DyninstAPI-${VERSION_STRING}")

if(BUILD_TARBALLS)
    find_package(Git)
    if(GIT_FOUND)
        if(EXISTS "${DYNINST_ROOT}/.git/")
            message(STATUS "Source tree is repository, building archive target")
            add_custom_target(package ALL)
            add_custom_command(
                TARGET package
                COMMAND ${GIT_EXECUTABLE} archive --prefix="${DYNINST_NAME}/"
                        --format=tar.gz -o "${CMAKE_BINARY_DIR}/${DYNINST_NAME}.tgz" HEAD
                WORKING_DIRECTORY ${DYNINST_ROOT}
                COMMENT "Packaging Dyninst")
        endif()
    endif()
endif()

if(BUILD_DOCS)
    add_custom_target(doc)
    set(LATEX_DOCS
        dynC_API
        dataflowAPI
        instructionAPI
        parseAPI
        patchAPI
        symtabAPI
        stackwalk)
    set(WORD_DOCS proccontrol dyninstAPI)
    add_custom_target(proccontrol-doc
                      DEPENDS ${CMAKE_SOURCE_DIR}/proccontrol/doc/proccontrol.pdf)
    add_custom_target(dyninstAPI-doc
                      DEPENDS ${CMAKE_SOURCE_DIR}/dyninstAPI/doc/dyninstAPI.pdf)
    foreach(COMPONENT ${WORD_DOCS})
        add_dependencies(doc ${COMPONENT}-doc)
        set_target_properties(${COMPONENT}-doc PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.pdf
            DESTINATION ${INSTALL_DOC_DIR}
            RENAME ${COMPONENT}-${VERSION_STRING}.pdf
            OPTIONAL)
    endforeach()

    find_package(LATEX)
    if(PDFLATEX_COMPILER)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/common/doc
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/common)
        foreach(COMPONENT ${LATEX_DOCS})
            file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${COMPONENT}/doc
                 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT})
            set(DEPS "")
            file(GLOB_RECURSE DEPS ${CMAKE_CURRENT_SOURCE_DIR}/${COMPONENT}/doc/*.tex
                 ${CMAKE_CURRENT_SOURCE_DIR}/common/doc/*.tex)
            add_custom_command(
                OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.aux
                DEPENDS ${DEPS}
                COMMAND ${PDFLATEX_COMPILER} ARGS -interaction=batchmode ${COMPONENT}.tex
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc
                COMMENT "Latex (first pass)")
            add_custom_command(
                OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.log
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.aux
                COMMAND ${PDFLATEX_COMPILER} ARGS -interaction=batchmode ${COMPONENT}.tex
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc
                COMMENT "Latex (second pass)")
            add_custom_target(
                ${COMPONENT}-doc
                echo
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.log)
            add_dependencies(doc ${COMPONENT}-doc)
            set_target_properties(${COMPONENT}-doc PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD
                                                              1)
            install(
                FILES ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.pdf
                DESTINATION ${INSTALL_DOC_DIR}
                RENAME ${COMPONENT}-${VERSION_STRING}.pdf
                OPTIONAL)
        endforeach()
    else()
        message(STATUS "LaTeX not found")
    endif()
endif()

# add_subdirectory(testsuite)

# Copy out all of the cmake files so they can be used by the Testsuite
install(DIRECTORY ${DYNINST_ROOT}/cmake DESTINATION ${INSTALL_CMAKE_DIR})
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    install(FILES ${PROJECT_BINARY_DIR}/CMakeCache.txt DESTINATION ${INSTALL_CMAKE_DIR})
endif()

include(DyninstInstall)

dyninst_print_features()
