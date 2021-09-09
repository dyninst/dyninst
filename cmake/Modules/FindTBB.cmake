# ======================================================================================================
# FindTBB.cmake
#
# Find TBB include directories and libraries.
#
# ----------------------------------------
#
# Use this module by invoking find_package with the form::
#
# find_package(TBB [major[.minor]] [EXACT] [QUIET]     # Minimum or EXACT version e.g.
# 2018.6 [REQUIRED]                          # Fail with error if TBB is not found
# [[COMPONENTS] [components...]]      # Required components [OPTIONAL_COMPONENTS
# components...] # Optional components )
#
# This module reads hints about search locations from variables::
#
# TBB_ROOT_DIR          - The base directory the of TBB installation. TBB_INCLUDE_DIR -
# The directory that contains the TBB headers files. TBB_LIBRARY           - The directory
# that contains the TBB library files. TBB_<library>_LIBRARY - The path of the TBB the
# corresponding TBB library. These libraries override the corresponding library search
# results. TBB_USE_DEBUG_BUILD   - Use the debug version of tbb libraries
#
# Environment variable aliases for TBB_ROOT_DIR:
#
# TBB_INSTALL_DIR TBBROOT LIBRARY_PATH
#
# This module will set the following variables:
#
# TBB_FOUND                     - If false, or undefined, TBB not found, or don’t want to
# use TBB. TBB_<component>_FOUND         - If False, optional <component> part of TBB
# sytem is not available. TBB_VERSION - The full version string TBB_VERSION_MAJOR - The
# major version TBB_VERSION_MINOR - The minor version TBB_INTERFACE_VERSION - The
# interface version number defined in tbb/tbb_stddef.h. TBB_<library>_LIBRARY_RELEASE -
# The path of the TBB release version of <library>. TBB_<library>_LIBRARY_DEBUG   - The
# path of the TBB debug version of <library>.
#
# The following varibles should be used to build and link with TBB:
#
# TBB_INCLUDE_DIRS        - The include directory for TBB. TBB_LIBRARY_DIRS - The library
# directory for TBB. TBB_LIBRARIES           - The libraries to link against to use TBB.
# TBB_LIBRARIES_RELEASE   - The release libraries to link against to use TBB.
# TBB_LIBRARIES_DEBUG     - The debug libraries to link against to use TBB.
# TBB_DEFINITIONS         - Definitions to use when compiling code that uses TBB.
# TBB_DEFINITIONS_RELEASE - Definitions to use when compiling release code that uses TBB.
# TBB_DEFINITIONS_DEBUG   - Definitions to use when compiling debug code that uses TBB.
#
# This module will also create the "TBB" target that may be used when building executables
# and libraries.
#
# Based on the version by Justus Calvin - Copyright (c) 2015
#
# ======================================================================================================

if(TBB_FOUND)
    return()
endif()

include(FindPackageHandleStandardArgs)

#
# Check the build type
#
if(NOT DEFINED TBB_USE_DEBUG_BUILD)
    if(CMAKE_BUILD_TYPE MATCHES "(Debug|DEBUG|debug)")
        set(TBB_BUILD_TYPE DEBUG)
    else()
        set(TBB_BUILD_TYPE RELEASE)
    endif()
elseif(TBB_USE_DEBUG_BUILD)
    set(TBB_BUILD_TYPE DEBUG)
else()
    set(TBB_BUILD_TYPE RELEASE)
endif()

#
# Set the TBB search directories
#

# Define search paths based on user input and environment variables
set(TBB_SEARCH_DIR ${TBB_ROOT_DIR} $ENV{TBB_INSTALL_DIR} $ENV{TBBROOT})

# Define the search directories based on the current platform
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TBB_DEFAULT_SEARCH_DIR "C:/Program Files/Intel/TBB"
                               "C:/Program Files (x86)/Intel/TBB")

    # Set the target architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(TBB_ARCHITECTURE "intel64")
    else()
        set(TBB_ARCHITECTURE "ia32")
    endif()

    # Set the TBB search library path search suffix based on the version of VC
    if(WINDOWS_STORE)
        set(TBB_LIB_PATH_SUFFIX "lib/${TBB_ARCHITECTURE}/vc11_ui")
    elseif(MSVC14)
        set(TBB_LIB_PATH_SUFFIX "lib/${TBB_ARCHITECTURE}/vc14")
    elseif(MSVC12)
        set(TBB_LIB_PATH_SUFFIX "lib/${TBB_ARCHITECTURE}/vc12")
    elseif(MSVC11)
        set(TBB_LIB_PATH_SUFFIX "lib/${TBB_ARCHITECTURE}/vc11")
    elseif(MSVC10)
        set(TBB_LIB_PATH_SUFFIX "lib/${TBB_ARCHITECTURE}/vc10")
    endif()

    # Add the library path search suffix for the VC independent version of TBB
    list(APPEND TBB_LIB_PATH_SUFFIX "lib/${TBB_ARCHITECTURE}/vc_mt")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # OS X
    set(TBB_DEFAULT_SEARCH_DIR "/opt/intel/tbb")

    # TODO: Check to see which C++ library is being used by the compiler.
    if(NOT ${CMAKE_SYSTEM_VERSION} VERSION_LESS 13.0)
        # The default C++ library on OS X 10.9 and later is libc++
        set(TBB_LIB_PATH_SUFFIX "lib/libc++" "lib")
    else()
        set(TBB_LIB_PATH_SUFFIX "lib")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux
    set(TBB_DEFAULT_SEARCH_DIR "/opt/intel/tbb")

    # TODO: Check compiler version to see the suffix should be <arch>/gcc4.1 or
    # <arch>/gcc4.1. For now, assume that the compiler is more recent than gcc 4.4.x or
    # later.
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(TBB_LIB_PATH_SUFFIX "lib/intel64/gcc4.4")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
        set(TBB_LIB_PATH_SUFFIX "lib/ia32/gcc4.4")
    endif()
endif()

#
# Find the TBB include dir
#
find_path(
    TBB_INCLUDE_DIRS tbb/tbb.h
    HINTS ${TBB_INCLUDE_DIRS} ${TBB_SEARCH_DIR}
    PATHS ${TBB_DEFAULT_SEARCH_DIR}
    PATH_SUFFIXES include)

#
# Set version strings
#
if(TBB_INCLUDE_DIRS)
    # Starting in 2020.1.1, tbb_stddef.h is replaced by version.h
    set(_version_files "${TBB_INCLUDE_DIRS}/tbb/tbb_stddef.h"
                       "${TBB_INCLUDE_DIRS}/tbb/version.h")
    foreach(f IN ITEMS ${_version_files})
        if(EXISTS ${f})
            set(_version_file ${f})
        endif()
    endforeach()
    unset(_version_files)

    file(READ ${_version_file} _tbb_version_file)
    string(REGEX REPLACE ".*#define TBB_VERSION_MAJOR ([0-9]+).*" "\\1" TBB_VERSION_MAJOR
                         "${_tbb_version_file}")
    string(REGEX REPLACE ".*#define TBB_VERSION_MINOR ([0-9]+).*" "\\1" TBB_VERSION_MINOR
                         "${_tbb_version_file}")
    string(REGEX REPLACE ".*#define TBB_INTERFACE_VERSION ([0-9]+).*" "\\1"
                         TBB_INTERFACE_VERSION "${_tbb_version_file}")

    # The TBB_VERSION_MINOR isn't necessarily changed for minor releases Hence, we need to
    # read the engineering versioning in TBB_INTERFACE_VERSION to get the minor version
    # correct
    if("${TBB_VERSION_MINOR}" STREQUAL "0")
        math(EXPR _tbb_iface_major_ver "${TBB_INTERFACE_VERSION} / 100")
        math(EXPR TBB_VERSION_MINOR
             "${TBB_INTERFACE_VERSION} - ${_tbb_iface_major_ver} * 100")
    endif()
    set(TBB_VERSION "${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR}")
endif()

#
# Find TBB components
#
if(TBB_VERSION VERSION_LESS 4.3)
    set(TBB_SEARCH_COMPOMPONENTS tbb_preview tbbmalloc tbb)
else()
    set(TBB_SEARCH_COMPOMPONENTS tbb_preview tbbmalloc_proxy tbbmalloc tbb)
endif()

set(TBB_LIBRARY_DIRS)

# Find each component
foreach(_comp ${TBB_SEARCH_COMPOMPONENTS})
    # message(STATUS "Searching for ${_comp}...") message(STATUS "Hints: ${TBB_LIBRARY}
    # ${TBB_SEARCH_DIR}")
    if(";${TBB_FIND_COMPONENTS};tbb;" MATCHES ";${_comp};")

        # Search for the libraries
        find_library(
            TBB_${_comp}_LIBRARY_RELEASE ${_comp}
            HINTS ${TBB_LIBRARY} ${TBB_SEARCH_DIR}
            PATHS ${TBB_DEFAULT_SEARCH_DIR} ENV LIBRARY_PATH
            PATH_SUFFIXES ${TBB_LIB_PATH_SUFFIX} lib_release)

        find_library(
            TBB_${_comp}_LIBRARY_DEBUG ${_comp}_debug
            HINTS ${TBB_LIBRARY} ${TBB_SEARCH_DIR}
            PATHS ${TBB_DEFAULT_SEARCH_DIR} ENV LIBRARY_PATH
            PATH_SUFFIXES ${TBB_LIB_PATH_SUFFIX} lib_debug)

        if(TBB_${_comp}_LIBRARY_DEBUG)
            list(APPEND TBB_LIBRARIES_DEBUG "${TBB_${_comp}_LIBRARY_DEBUG}")
            # message(STATUS "Found ${TBB_${_comp}_LIBRARY_DEBUG}")
        endif()
        if(TBB_${_comp}_LIBRARY_RELEASE)
            list(APPEND TBB_LIBRARIES_RELEASE "${TBB_${_comp}_LIBRARY_RELEASE}")
            # message(STATUS "Found ${TBB_${_comp}_LIBRARY_RELEASE}")
        endif()
        if(TBB_${_comp}_LIBRARY_${TBB_BUILD_TYPE} AND NOT TBB_${_comp}_LIBRARY)
            set(TBB_${_comp}_LIBRARY "${TBB_${_comp}_LIBRARY_${TBB_BUILD_TYPE}}")
        endif()

        if(TBB_${_comp}_LIBRARY AND EXISTS "${TBB_${_comp}_LIBRARY}")
            set(TBB_${_comp}_FOUND TRUE)
        else()
            set(TBB_${_comp}_FOUND FALSE)
        endif()

        # Mark internal variables as advanced
        mark_as_advanced(TBB_${_comp}_LIBRARY_RELEASE)
        mark_as_advanced(TBB_${_comp}_LIBRARY_DEBUG)
        mark_as_advanced(TBB_${_comp}_LIBRARY)

        # Save the directory names for each library component
        if(TBB_USE_DEBUG_BUILD)
            get_filename_component(_dir ${TBB_${_comp}_LIBRARY_DEBUG} DIRECTORY)
        else()
            get_filename_component(_dir ${TBB_${_comp}_LIBRARY_RELEASE} DIRECTORY)
        endif()
        list(APPEND TBB_LIBRARY_DIRS ${_dir})
    endif()
endforeach()

#
# Set compile flags and libraries
#
set(TBB_DEFINITIONS_RELEASE "")
set(TBB_DEFINITIONS_DEBUG "-DTBB_USE_DEBUG=1")

if(TBB_LIBRARIES_${TBB_BUILD_TYPE})
    set(TBB_DEFINITIONS "${TBB_DEFINITIONS_${TBB_BUILD_TYPE}}")
    set(TBB_LIBRARIES "${TBB_LIBRARIES_${TBB_BUILD_TYPE}}")
elseif(TBB_LIBRARIES_RELEASE)
    set(TBB_DEFINITIONS "${TBB_DEFINITIONS_RELEASE}")
    set(TBB_LIBRARIES "${TBB_LIBRARIES_RELEASE}")
elseif(TBB_LIBRARIES_DEBUG)
    set(TBB_DEFINITIONS "${TBB_DEFINITIONS_DEBUG}")
    set(TBB_LIBRARIES "${TBB_LIBRARIES_DEBUG}")
endif()

find_package_handle_standard_args(
    TBB
    REQUIRED_VARS TBB_INCLUDE_DIRS TBB_LIBRARIES
    HANDLE_COMPONENTS
    VERSION_VAR TBB_VERSION)

#
# Create targets
#
if(NOT CMAKE_VERSION VERSION_LESS 3.0 AND TBB_FOUND)
    add_library(TBB::TBB SHARED IMPORTED)
    set_target_properties(
        TBB::TBB PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${TBB_INCLUDE_DIRS}
                            IMPORTED_LOCATION ${TBB_LIBRARIES})
    if(TBB_LIBRARIES_RELEASE AND TBB_LIBRARIES_DEBUG)
        set_target_properties(
            TBB::TBB
            PROPERTIES INTERFACE_COMPILE_DEFINITIONS "$<$<CONFIG:Debug>:TBB_USE_DEBUG=1>"
                       IMPORTED_LOCATION_DEBUG ${TBB_LIBRARIES_DEBUG}
                       IMPORTED_LOCATION_RELWITHDEBINFO ${TBB_LIBRARIES_DEBUG}
                       IMPORTED_LOCATION_RELEASE ${TBB_LIBRARIES_RELEASE}
                       IMPORTED_LOCATION_MINSIZEREL ${TBB_LIBRARIES_RELEASE})
    elseif(TBB_LIBRARIES_RELEASE)
        set_target_properties(TBB::TBB PROPERTIES IMPORTED_LOCATION
                                                  ${TBB_LIBRARIES_RELEASE})
    else()
        set_target_properties(TBB::TBB PROPERTIES IMPORTED_LOCATION
                                                  ${TBB_LIBRARIES_DEBUG})
    endif()
endif()

mark_as_advanced(TBB_INCLUDE_DIRS TBB_LIBRARIES TBB_LIBRARY_DIRS)

unset(TBB_ARCHITECTURE)
unset(TBB_BUILD_TYPE)
unset(TBB_LIB_PATH_SUFFIX)
unset(TBB_DEFAULT_SEARCH_DIR)
