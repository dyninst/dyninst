include_guard(DIRECTORY)

set(DYNINST_SOVERSION "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}")

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
    set(CMAKE_CONFIGURATION_TYPES
        "${CMAKE_CONFIGURATION_TYPES}"
        CACHE
            STRING
            "Reset the available configurations to exclude MinSizeRel and RelWithDebugInfo"
            FORCE)
endif()

if(LIGHTWEIGHT_SYMTAB)
    set(SYMREADER symLite)
else()
    set(SYMREADER symtabAPI)
endif()

include(CMakeParseArguments)
include(DyninstUtilities)

# create a dummy target so one can do things like:
#
# if(NOT UNIX) add_library(Dyninst::dynElf ALIAS dynDummy) return() endif()
#
# and, later, just "link" to Dyninst::dynElf even though it doesn't really exist
if(NOT TARGET dynDummy)
    dyninst_add_interface_library(dynDummy "Dummy target for alias libraries")
endif()

dyninst_add_interface_library(dynDL "CMAKE_DL_LIBS")
dyninst_add_interface_library(dynPthread "Pthread library")

if(UNIX)
    target_link_libraries(dynDL INTERFACE ${CMAKE_DL_LIBS})
    target_link_libraries(dynPthread INTERFACE pthread)
endif()

dyninst_add_interface_library(dynWinPsapi "Windows psapi")
dyninst_add_interface_library(dynWinWs2_32 "Windows ws2_32")
dyninst_add_interface_library(dynWinShlwapi "Windows shlwapi")
dyninst_add_interface_library(dynWinDbgHelp "Windows dbghelp")
dyninst_add_interface_library(dynWinImageHelp "Windows imagehlp")

if(WIN32)
    target_link_libraries(dynWinDbgHelp INTERFACE dbghelp)
    target_link_libraries(dynWinImageHelp INTERFACE imagehlp)
    target_link_libraries(dynWinPsapi INTERFACE psapi)
    target_link_libraries(dynWinShlwapi INTERFACE shlwapi)
    target_link_libraries(dynWinWs2_32 INTERFACE ws2_32)
endif()

function(dyninst_library TARG_NAME)
    # boolean options
    set(_boolean_opts BUILD_SHARED BUILD_STATIC OPENMP)
    # options taking single value
    set(_single_val_opts DESTINATION DEFAULT_VISIBILITY)
    # options taking multiple values
    set(_multi_val_opts
        HEADERS
        SOURCES
        DEPENDS
        INCLUDE_DIRECTORIES # NOT affected by DEFAULT_VISIBILITY
        COMPILE_DEFINITIONS # affected by DEFAULT_VISIBILITY
        COMPILE_FEATURES # affected by DEFAULT_VISIBILITY
        COMPILE_OPTIONS # affected by DEFAULT_VISIBILITY
        LINK_LIBRARIES # affected by DEFAULT_VISIBILITY
        LINK_DIRECTORIES # affected by DEFAULT_VISIBILITY
        LINK_OPTIONS # affected by DEFAULT_VISIBILITY
        PROPERTIES)

    macro(_dyninst_library_set_default _VAR)
        if(NOT DEFINED ${_VAR} OR "${${_VAR}}" STREQUAL "")
            set(${_VAR} ${ARGN})
        endif()
    endmacro()

    cmake_parse_arguments(TARG "${_boolean_opts}" "${_single_val_opts}"
                          "${_multi_val_opts}" ${ARGN})

    # all the targets
    set(ADDED_TARGETS)

    # local headers
    file(GLOB _DEFAILT_TARG_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/h/*.h"
         "${CMAKE_CURRENT_BINARY_DIR}/h/*.h")

    # set default values
    _dyninst_library_set_default(TARG_DEFAULT_VISIBILITY PUBLIC)
    _dyninst_library_set_default(TARG_DESTINATION ${INSTALL_LIB_DIR})
    _dyninst_library_set_default(TARG_HEADERS ${_DEFAILT_TARG_HEADERS})
    _dyninst_library_set_default(BUILD_SHARED_LIBS ON)
    _dyninst_library_set_default(BUILD_STATIC_LIBS OFF)

    if(BUILD_SHARED_LIBS OR TARG_BUILD_SHARED)
        add_library(${TARG_NAME} SHARED)
        add_library(${PROJECT_NAME}::${TARG_NAME} ALIAS ${TARG_NAME})
        list(APPEND ADDED_TARGETS ${TARG_NAME})
    endif()

    if(BUILD_STATIC_LIBS OR TARG_BUILD_STATIC)
        add_library(${TARG_NAME}-static STATIC)
        add_library(${PROJECT_NAME}::${TARG_NAME}-static ALIAS ${TARG_NAME}-static)
        list(APPEND ADDED_TARGETS ${TARG_NAME}-static)
        if(NOT BUILD_SHARED_LIBS AND NOT TARGET ${PROJECT_NAME}::${TARG_NAME})
            add_library(${PROJECT_NAME}::${TARG_NAME} ALIAS ${TARG_NAME}-static)
        endif()
    endif()

    # Make sure OpenMP::OpenMP_{C,CXX} targets exist
    if(USE_OpenMP AND NOT OpenMP_FOUND)
        find_package(OpenMP REQUIRED)
    endif()

    # Handle internal third-party library builds
    if(NOT TARGET dyninst-external-libraries)
        add_custom_target(dyninst-external-libraries)
        foreach(_EXTERNAL_DEP Boost TBB ElfUtils LibIberty)
            if(TARGET ${_EXTERNAL_DEP}-External)
                add_dependencies(dyninst-external-libraries ${_EXTERNAL_DEP}-External)
            endif()
        endforeach()
    endif()

    # Handle formatting
    file(GLOB_RECURSE _format_sources "${CMAKE_CURRENT_LIST_DIR}/h/*.h"
         "${CMAKE_CURRENT_LIST_DIR}/src/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
         "${CMAKE_CURRENT_LIST_DIR}/src/*.C")
    foreach(_SRC ${TARG_SOURCES} ${TARG_HEADERS})
        if(NOT EXISTS ${_SRC} OR NOT IS_ABSOLUTE ${_SRC})
            set(_SRC ${CMAKE_CURRENT_LIST_DIR}/${_SRC})
        endif()
        list(APPEND _format_sources ${_SRC})
    endforeach()
    list(REMOVE_DUPLICATES _format_sources)
    dyninst_add_source_format_target(${TARG_NAME} ${_format_sources})

    foreach(_target ${ADDED_TARGETS})

        foreach(_DEP ${TARG_DEPENDS})
            if(TARGET ${_DEP}) # dependencies must always be a target
                add_dependencies(${_target} ${_DEP})
            endif()
        endforeach()

        # this has no effect if there are no TPLs to build
        add_dependencies(${_target} dyninst-external-libraries)

        target_sources(${_target} PRIVATE ${TARG_SOURCES} ${TARG_HEADERS})
        target_include_directories(${_target} BEFORE PRIVATE ${TARG_INCLUDE_DIRECTORIES})
        # target_include_directories above is written this way to combat subfolders which
        # have headers with same name, e.g. common/h/util.h and parseAPI/src/util.h. With
        # BEFORE + PRIVATE, there is a way to prioritize include paths in the build tree
        # and not have that BEFORE propagate because of the PRIVATE. See example in
        # parseAPI/CMakeLists.txt. Furthermore, PRIVATE forces it be explicit when a
        # subfolder depends on a header in another subfolder's src folder, e.g.
        # dynC_API/CMakeLists.txt
        target_compile_definitions(${_target} ${TARG_DEFAULT_VISIBILITY}
                                              ${TARG_COMPILE_DEFINITIONS})
        target_compile_features(${_target} ${TARG_DEFAULT_VISIBILITY}
                                           ${TARG_COMPILE_FEATURES})
        target_compile_options(${_target} ${TARG_DEFAULT_VISIBILITY}
                               ${TARG_COMPILE_OPTIONS})
        target_link_directories(${_target} ${TARG_DEFAULT_VISIBILITY}
                                ${TARG_LINK_DIRECTORIES})
        target_link_options(${_target} ${TARG_DEFAULT_VISIBILITY} ${TARG_LINK_OPTIONS})

        # default link visibility
        set(_LINK_VISIBILITY ${TARG_DEFAULT_VISIBILITY})

        # special handling of target_link_libraries to make sure static libraries link to
        # static targets when a static target exists
        foreach(_LINK_LIB ${TARG_LINK_LIBRARIES})
            # if encounter a visibility entry, store it and more to next iteration
            if("${_LINK_LIB}" MATCHES "^(INTERFACE|PUBLIC|PRIVATE)$")
                set(_LINK_VISIBILITY ${_LINK_LIB})
                continue()
            endif()
            # modify the variable to point to static target if it exists
            if("${_target}" MATCHES "static" AND TARGET ${_LINK_LIB}-static)
                set(_LINK_LIB ${_LINK_LIB}-static)
            endif()
            if(DYNINST_DEBUG_CMAKE)
                dyninst_message(
                    STATUS
                    "target_link_libraries(${_target} ${_LINK_VISIBILITY} ${_LINK_LIB})")
            endif()
            target_link_libraries(${_target} ${_LINK_VISIBILITY} ${_LINK_LIB})
        endforeach()

        target_link_libraries(${_target} PRIVATE Dyninst::dynCapArchDef)
        if(USE_OpenMP AND TARG_OPENMP)
            target_link_libraries(${_target} PRIVATE OpenMP::OpenMP_C OpenMP::OpenMP_CXX)
        endif()

        set_target_properties(
            ${_target}
            PROPERTIES OUTPUT_NAME ${TARG_NAME}
                       SOVERSION ${DYNINST_SOVERSION} # ignored for static libs
                       VERSION ${DYNINST_VERSION} # ignored for static libs
                       PUBLIC_HEADER "${TARG_HEADERS}"
                       CLEAN_DIRECT_OUTPUT 1
                       LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                       ${TARG_PROPERTIES} # generic variable
            )

        add_custom_target(
            ${_target}-install
            DEPENDS ${_target}
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake)

        install(
            TARGETS ${_target}
            EXPORT ${TARG_NAME}Targets
            COMPONENT ${TARG_NAME}
            RUNTIME DESTINATION ${TARG_DESTINATION}
            LIBRARY DESTINATION ${TARG_DESTINATION}
            ARCHIVE DESTINATION ${TARG_DESTINATION}
            PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
    endforeach()

    if(ADDED_TARGETS)
        install(
            EXPORT ${TARG_NAME}Targets
            NAMESPACE ${PROJECT_NAME}::
            DESTINATION ${INSTALL_CMAKE_DIR})

        export(
            TARGETS ${ADDED_TARGETS}
            NAMESPACE ${PROJECT_NAME}::
            FILE ${PROJECT_BINARY_DIR}/build-tree/${TARG_NAME}Targets.cmake
            EXPORT_LINK_INTERFACE_LIBRARIES)
    endif()

    dyninst_append_property_list(ALL_DYNINST_TARGETS ${TARG_NAME})
endfunction()

include(DyninstPlatform)
include(DyninstCapArchDef)
include(DyninstVisibility)
include(DyninstWarnings)
include(DyninstOptions)
include(DyninstOptimization)
include(DyninstEndian)
include(DyninstInstallDirs)

# provide an absolute path (if needed) but do not rewrite the with absolute path because
# CPack only bundles install commands which use relative paths
foreach(p BIN LIB INCLUDE CMAKE)
    set(INSTALL_${p}_FULL_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}_DIR}")
endforeach()

if(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 19)
        add_compile_definitions(_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
    else()
        add_compile_definitions(snprintf=_snprintf)
    endif()
endif()

# set default configuration type
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE
        RelWithDebInfo
        CACHE STRING
              "Choose the build type (None, Debug, Release, RelWithDebInfo, MinSizeRel)"
              FORCE)
endif()
