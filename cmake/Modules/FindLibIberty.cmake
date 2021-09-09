# ========================================================================================
# FindLibIberty.cmake
#
# Find LibIberty include dirs and libraries
#
# ----------------------------------------
#
# Use this module by invoking find_package with the form::
#
# find_package(LibIberty [REQUIRED]             # Fail with error if LibIberty is not
# found )
#
# This module reads hints about search locations from variables::
#
# LibIberty_ROOT_DIR      - Base directory the of LibIberty installation
# LibIberty_LIBRARYDIR    - Hint directory that contains the LibIberty library files
# IBERTY_LIBRARIES        - Alias for LibIberty_LIBRARIES (backwards compatibility only)
# LibIberty_INCLUDEDIR    - Hint directory that contains the libiberty headers files
#
# and saves search results persistently in CMake cache entries::
#
# LibIberty_FOUND         - True if headers and requested libraries were found
# IBERTY_FOUND            - Alias for LibIberty_FOUND (backwards compatibility only)
# LibIberty_INCLUDE_DIRS  - libiberty include directories LibIberty_LIBRARY_DIRS  - Link
# directories for LibIberty libraries LibIberty_LIBRARIES     - LibIberty library files
# IBERTY_LIBRARIES        - Alias for LibIberty_LIBRARIES (backwards compatibility only)
#
# ========================================================================================

cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

# Keep the semantics of IBERTY_LIBRARIES for backward compatibility NB: If both are
# specified, LibIberty_LIBRARIES is ignored
if(NOT "${IBERTY_LIBRARIES}" STREQUAL "")
    set(LibIberty_LIBRARIES ${IBERTY_LIBRARIES})
endif()

include(DyninstSystemPaths)

# Non-standard subdirectories to search
set(_path_suffixes libiberty iberty)

find_path(
    LibIberty_INCLUDE_DIRS
    NAMES libiberty.h
    HINTS ${LibIberty_ROOT_DIR} ${LibIberty_ROOT_DIR}/include ${LibIberty_INCLUDEDIR}
    PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
    PATH_SUFFIXES ${_path_suffixes}
    DOC "LibIberty include directories")

# iberty_pic is for Debian <= wheezy
find_library(
    LibIberty_LIBRARIES
    NAMES iberty_pic iberty
    HINTS ${LibIberty_ROOT_DIR} ${LibIberty_LIBRARYDIR} ${IBERTY_LIBRARIES}
    PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
    PATH_SUFFIXES ${_path_suffixes})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibIberty
    FOUND_VAR LibIberty_FOUND
    REQUIRED_VARS LibIberty_INCLUDE_DIRS LibIberty_LIBRARIES)

# For backwards compatibility only
set(IBERTY_FOUND ${LibIberty_FOUND})

if(LibIberty_FOUND)
    foreach(l ${LibIberty_LIBRARIES})
        get_filename_component(_dir ${l} DIRECTORY)
        if(NOT "${_dir}" IN_LIST LibIberty_LIBRARY_DIRS)
            list(APPEND LibIberty_LIBRARY_DIRS ${_dir})
        endif()
    endforeach()

    add_library(LibIberty::LibIberty INTERFACE IMPORTED)
    target_include_directories(LibIberty::LibIberty INTERFACE ${LibIberty_INCLUDE_DIRS})
    target_link_libraries(LibIberty::LibIberty INTERFACE ${LibIberty_LIBRARIES})

    # For backwards compatibility only
    set(IBERTY_LIBRARIES ${LibIberty_LIBRARIES})
endif()
