#========================================================================================
# FindPeParse.cmake
#
# Find pe-parse include dirs and libraries
#
#		----------------------------------------
#
# Use this module by invoking find_package with the form::
#
#  find_package(PeParse
#    [version] [EXACT]      # Minimum or EXACT version e.g. 0.173
#    [REQUIRED]             # Fail with error if pe-parse is not found
#  )
#
# This module reads hints about search locations from variables::
#
#	PeParse_ROOT_DIR	- Base directory the of pe-parse installation
#	PeParse_INCLUDEDIR	- Hint directory that contains the pe-parse headers files
#	PeParse_LIBRARYDIR	- Hint directory that contains the pe-parse library files
#
# and saves search results persistently in CMake cache entries::
#
#	PeParse_FOUND		- True if headers and requested libraries were found
#	PeParse_INCLUDE_DIRS 	- pe-parse include directories
#	PeParse_LIBRARY_DIRS	- Link directories for pe-parse libraries
#	PeParse_LIBRARIES	- pe-parse library files
#
#========================================================================================
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

# Non-standard subdirectories to search
# set(_path_suffixes pe-parse)

if(PeParse_NO_SYSTEM_PATHS)
    set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

find_path(
    PeParse_INCLUDE_DIRS
    NAMES pe-parse/parse.h
    HINTS ${PeParse_ROOT_DIR}/include ${PeParse_ROOT_DIR} ${PeParse_INCLUDEDIR}
    PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
    PATH_SUFFIXES ${_path_suffixes}
    DOC "pe-parse include directories")
mark_as_advanced(PeParse_INCLUDE_DIRS)

message(STATUS "PeParse_INCLUDE_DIRS: ${PrParse_INCLUDE_DIRS}")

find_library(
    PeParse_LIBRARIES
    NAMES libpe-parse.so
    HINTS ${PeParse_ROOT_DIR}/lib ${PeParse_ROOT_DIR} ${PeParse_LIBRARYDIR}
    PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
    PATH_SUFFIXES ${_path_suffixes})
mark_as_advanced(PeParse_LIBRARIES)

if(NOT PeParse_LIBRARIES OR NOT PeParse_INCLUDE_DIRS)
    set(PeParse_FOUND FALSE)
    return()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    PeParse
    FOUND_VAR PeParse_FOUND
    REQUIRED_VARS PeParse_LIBRARIES PeParse_INCLUDE_DIRS
    VERSION_VAR PeParse_VERSION)

# Export cache variables
if(NOT TARGET PeParse::PeParse)
    add_library(PeParse::PeParse UNKNOWN IMPORTED)
    if(PeParse_FOUND)
        set_target_properties(
            PeParse::PeParse
            PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${PeParse_INCLUDE_DIRS}"
                       IMPORTED_LOCATION "${PeParse_LIBRARIES}")
    endif()
endif()
