#[=======================================================================[.rst:
FindLibDebuginfod
-----------------

Find libdebuginfod, the elfutils library to query debuginfo files from debuginfod servers.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``LibDebuginfod::LibDebuginfod``
  The libdebuginfod library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``LibDebuginfod_INCLUDE_DIRS``
  where to find debuginfod.h, etc.
``LibDebuginfod_LIBRARIES``
  the libraries to link against to use libdebuginfod.
``LibDebuginfod_FOUND``
  If false, do not try to use libdebuginfod.
``LibDebuginfod_VERSION``
  the version of the libdebuginfod library found

#]=======================================================================]
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(LibDebuginfod_FIND_QUIETLY)
    set(_quiet "QUIET")
endif()

if(NOT "x${LibDebuginfod_FIND_VERSION}" STREQUAL "x")
    set(_version ">=${LibDebuginfod_FIND_VERSION}")
endif()

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LIBDEBUGINFOD ${_quiet} "libdebuginfod${_version}")
endif()

if(PC_LIBDEBUGINFOD_FOUND)
		# FindPkgConfig sometimes gets the include dir wrong
		if("x${PC_LIBDEBUGINFOD_INCLUDE_DIRS}" STREQUAL "x")
			pkg_get_variable(PC_LIBDEBUGINFOD_INCLUDE_DIRS libdebuginfod includedir)
		endif()
		
    set(LibDebuginfod_INCLUDE_DIRS
        ${PC_LIBDEBUGINFOD_INCLUDE_DIRS}
        CACHE PATH "")
    set(LibDebuginfod_LIBRARIES
        ${PC_LIBDEBUGINFOD_LINK_LIBRARIES}
        CACHE PATH "")
    set(LibDebuginfod_VERSION
        ${PC_LIBDEBUGINFOD_VERSION}
        CACHE STRING "")
else()
    find_path(
        LibDebuginfod_INCLUDE_DIRS
        NAMES debuginfod.h
        PATH_SUFFIXES elfutils)

    find_library(
        LibDebuginfod_LIBRARIES
        NAMES libdebuginfod debuginfod
        PATH_SUFFIXES elfutils)

    if(EXISTS "${LibDebuginfod_INCLUDE_DIRS}/version.h")
        file(STRINGS "${LibDebuginfod_INCLUDE_DIRS}/version.h" _version_line
             REGEX "^#define _ELFUTILS_VERSION[ \t]+[0-9]+")
        string(REGEX MATCH "[0-9]+" _version "${_version_line}")
        if(NOT "x${_version}" STREQUAL "x")
            set(LibDebuginfod_VERSION "0.${_version}")
        endif()
        unset(_version_line)
        unset(_version)
    endif()

    if("x${LibDebuginfod_VERSION}" STREQUAL "x")
        message(FATAL_ERROR "Unable to find version for libdebuginfod")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibDebuginfod
    FOUND_VAR LibDebuginfod_FOUND
    REQUIRED_VARS LibDebuginfod_LIBRARIES LibDebuginfod_INCLUDE_DIRS
    VERSION_VAR LibDebuginfod_VERSION)

if(LibDebuginfod_FOUND)
    mark_as_advanced(LibDebuginfod_INCLUDE_DIR)
    mark_as_advanced(LibDebuginfod_LIBRARIES)

    if(NOT TARGET LibDebuginfod::LibDebuginfod)
        add_library(LibDebuginfod::LibDebuginfod UNKNOWN IMPORTED)
        set_target_properties(
            LibDebuginfod::LibDebuginfod PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                    "${LibDebuginfod_INCLUDE_DIRS}")

        set_target_properties(
            LibDebuginfod::LibDebuginfod
            PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION
                                                             "${LibDebuginfod_LIBRARIES}")
    endif()
endif()

unset(_quiet)
unset(_version)
