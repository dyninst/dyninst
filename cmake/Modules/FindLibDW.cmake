#[=======================================================================[.rst:
FindLibDW
---------

Find libdw, the elfutils library for DWARF data and ELF file or process inspection.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``LibDW::LibDW``
  The libdw library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``LibDW_INCLUDE_DIRS``
  where to find libdw.h, etc.
``LibDW_LIBRARIES``
  the libraries to link against to use libdw.
``LibDW_FOUND``
  If false, do not try to use libdw.
``LibDW_VERSION``
  the version of the libdw library found

#]=======================================================================]
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(LibDW_FIND_QUIETLY)
	set(_quiet "QUIET")
endif()

if(NOT "x${LibDW_FIND_VERSION}" STREQUAL "x")
	set(_version ">=${LibDW_FIND_VERSION}")
endif()

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LIBDW ${_quiet} "libdw${_version}")
endif()

find_path(
    LibDW_INCLUDE_DIR
    NAMES libdw.h
    HINTS ${PC_LIBDW_INCLUDE_DIRS}
    PATH_SUFFIXES elfutils)
mark_as_advanced(LibDW_INCLUDE_DIR)

find_library(
    LibDW_LIBRARY
    NAMES libdw dw
    HINTS ${PC_LIBDW_LIBRARY_DIRS}
    PATH_SUFFIXES elfutils)
mark_as_advanced(LibDW_LIBRARY)

if(EXISTS "${LibDW_INCLUDE_DIR}/version.h")
    file(STRINGS "${LibDW_INCLUDE_DIR}/version.h" _version_line
         REGEX "^#define _ELFUTILS_VERSION[ \t]+[0-9]+")
    string(REGEX MATCH "[0-9]+" _version "${_version_line}")
    if(NOT "x${_version}" STREQUAL "x")
        set(LibDW_VERSION "0.${_version}")
    endif()
    unset(_version_line)
    unset(_version)
elseif(PC_LIBDW_FOUND)
    set(LibDW_VERSION "${PC_LIBDW_VERSION}")
endif()

if("x${LibDW_VERSION}" STREQUAL "x")
    message(FATAL_ERROR "Unable to find version for libdw")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibDW
    FOUND_VAR LibDW_FOUND
    REQUIRED_VARS LibDW_LIBRARY LibDW_INCLUDE_DIR
    VERSION_VAR LibDW_VERSION)

if(LibDW_FOUND)
    set(LibDW_INCLUDE_DIRS ${LibDW_INCLUDE_DIR})
    set(LibDW_LIBRARIES ${LibDW_LIBRARY})

    if(NOT TARGET LibDW::LibDW)
        add_library(LibDW::LibDW UNKNOWN IMPORTED)
        set_target_properties(LibDW::LibDW PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                      "${LibDW_INCLUDE_DIRS}")

        set_target_properties(
            LibDW::LibDW PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                    IMPORTED_LOCATION "${LibDW_LIBRARIES}")
    endif()
endif()
