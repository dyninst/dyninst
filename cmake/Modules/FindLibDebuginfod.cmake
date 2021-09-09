# ========================================================================================
# FindDebuginfod
# -----------
#
# Find debuginfod library and headers
#
# The module defines the following variables:
#
# This module reads hints about search locations from variables::
#
# LibDebuginfod_ROOT_DIR         - Base directory the of libdebuginfod installation
# LibDebuginfod_INCLUDEDIR       - Hint directory that contains the libdebuginfod headers
# files LibDebuginfod_LIBRARYDIR       - Hint directory that contains the libdebuginfod
# library files
#
# and saves search results persistently in CMake cache entries::
#
# LibDebuginfod_FOUND            - True if headers and requested libraries were found
# LibDebuginfod_INCLUDE_DIRS     - libdebuginfod include directories
# LibDebuginfod_LIBRARY_DIRS     - Link directories for libdebuginfod libraries
# LibDebuginfod_LIBRARIES        - libdebuginfod library files
#
# Utilize package config (e.g. /usr/lib64/pkgconfig/libdebuginfod.pc) to fetch version
# information.
#
# ========================================================================================

find_package(PkgConfig QUIET)
pkg_check_modules(PC_Debuginfod QUIET REQUIRED libdebuginfod>=${ElfUtils_MIN_VERSION})
set(LibDebuginfod_VERSION "${PC_Debuginfod_VERSION}")

find_path(
    LibDebuginfod_INCLUDE_DIRS
    NAMES debuginfod.h
    HINTS ${PC_Debuginfod_INCLUDEDIR} ${PC_Debuginfod_INCLUDE_DIRS}
          ${LibDebuginfod_ROOT_DIR}/include ${LibDebuginfod_ROOT_DIR}
          ${LibDebuginfod_INCLUDEDIR}
    PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
    PATH_SUFFIXES ${_path_suffixes}
    DOC "libdebuginfod include directories")

find_library(
    LibDebuginfod_LIBRARIES
    NAMES libdebuginfod.so.1 libdebuginfod.so
    HINTS ${PC_Debuginfod_LIBDIR} ${PC_Debuginfod_LIBRARY_DIRS}
          ${LibDebuginfod_ROOT_DIR}/lib ${LibDebuginfod_ROOT_DIR}
          ${LibDebuginfod_LIBRARYDIR}
    PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
    PATH_SUFFIXES ${_path_suffixes})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibDebuginfod
    FOUND_VAR LibDebuginfod_FOUND
    REQUIRED_VARS LibDebuginfod_INCLUDE_DIRS LibDebuginfod_LIBRARIES
    VERSION_VAR LibDebuginfod_VERSION)

if(LibDebuginfod_FOUND)
    set(LibDebuginfod_INCLUDE_DIRS ${LibDebuginfod_INCLUDE_DIRS})
    set(LibDebuginfod_LIBRARIES ${LibDebuginfod_LIBRARIES})
    get_filename_component(_debuginfod_dir ${LibDebuginfod_LIBRARIES} DIRECTORY)
    set(LibDebuginfod_LIBRARY_DIRS ${_debuginfod_dir} "${_debuginfod_dir}/elfutils")

    add_library(LibDebuginfod::LibDebuginfod INTERFACE IMPORTED)
    target_include_directories(LibDebuginfod::LibDebuginfod
                               INTERFACE ${LibDebuginfod_INCLUDE_DIR})
    target_link_directories(LibDebuginfod::LibDebuginfod INTERFACE
                            ${LibDebuginfod_LIBRARY_DIRS})
    target_link_libraries(LibDebuginfod::LibDebuginfod
                          INTERFACE ${LibDebuginfod_LIBRARIES})
endif()
