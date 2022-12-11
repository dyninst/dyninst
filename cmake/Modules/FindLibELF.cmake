#[=======================================================================[.rst:
FindLibELF
----------

Find libelf, the elfutils library to read and write ELF files.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``LibELF::LibELF``
  The libelf library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``LibELF_INCLUDE_DIRS``
  where to find libelf.h, etc.
``LibELF_LIBRARIES``
  the libraries to link against to use libelf.
``LibELF_FOUND``
  If false, do not try to use libelf.
``LibELF_VERSION``
  the version of the libelf library found

#]=======================================================================]
cmake_policy(SET CMP0074 NEW)  # Use <Package>_ROOT

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LIBELF QUIET libelf)
endif()

find_path(
    LibELF_INCLUDE_DIR
    NAMES libelf.h
    HINTS ${PC_LIBELF_INCLUDE_DIRS}
    PATH_SUFFIXES elfutils)
mark_as_advanced(LibELF_INCLUDE_DIR)

find_library(
    LibELF_LIBRARY
    NAMES libelf elf
    HINTS ${PC_LIBELF_LIBRARY_DIRS}
    PATH_SUFFIXES elfutils)
mark_as_advanced(LibELF_LIBRARY)

macro(_check_libelf_version _file)
    file(STRINGS ${_file} _version_line REGEX "^#define _ELFUTILS_VERSION[ \t]+[0-9]+")
    string(REGEX MATCH "[0-9]+" _version "${_version_line}")
    if(NOT "x${_version}" STREQUAL "x")
        set(LibELF_VERSION "0.${_version}")
    endif()
    unset(_version_line)
    unset(_version)
endmacro()

if(EXISTS "${LibELF_INCLUDE_DIR}/version.h")
    _check_libelf_version("${LibELF_INCLUDE_DIR}/version.h")
elseif(EXISTS "${LibELF_INCLUDE_DIR}/elfutils/version.h")
    _check_libelf_version("${LibELF_INCLUDE_DIR}/elfutils/version.h")
elseif(PC_LIBELF_FOUND)
    set(LibELF_VERSION "${PC_LIBELF_VERSION}")
endif()

if("x${LibELF_VERSION}" STREQUAL "x")
    message(FATAL_ERROR "Unable to find version for libelf")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibELF
    FOUND_VAR LibELF_FOUND
    REQUIRED_VARS LibELF_LIBRARY LibELF_INCLUDE_DIR
    VERSION_VAR LibELF_VERSION)

if(LibELF_FOUND)
    set(LibELF_INCLUDE_DIRS ${LibELF_INCLUDE_DIR})
    set(LibELF_LIBRARIES ${LibELF_LIBRARY})

    if(NOT TARGET LibELF::LibELF)
        add_library(LibELF::LibELF UNKNOWN IMPORTED)
        set_target_properties(LibELF::LibELF PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                        "${LibELF_INCLUDE_DIRS}")

        set_target_properties(
            LibELF::LibELF PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                      IMPORTED_LOCATION "${LibELF_LIBRARIES}")
    endif()
endif()
