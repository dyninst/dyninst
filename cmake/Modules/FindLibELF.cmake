#[=======================================================================[.rst:
FindLibELF
----------

Find libelf, the elfutils library to read and write ELF files.

Variables that affect this module

``LibELF_NO_SYSTEM_PATHS``
  If `True`, no system paths are searched.

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
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(LibELF_NO_SYSTEM_PATHS)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

# There is no way to tell pkg-config to ignore directories, so disable it
if(NOT LibELF_NO_SYSTEM_PATHS)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    if(NOT "x${LibELF_FIND_VERSION}" STREQUAL "x")
      set(_version ">=${LibELF_FIND_VERSION}")
    endif()
    if(LibELF_FIND_QUIETLY)
      set(_quiet "QUIET")
    endif()

    pkg_check_modules(PC_LIBELF ${_quiet} "libelf${_version}")
    unset(_version)
    unset(_quiet)
  endif()
endif()

if(PC_LIBELF_FOUND)
  # FindPkgConfig sometimes gets the include dir wrong
  if("x${PC_LIBELF_INCLUDE_DIRS}" STREQUAL "x")
    pkg_get_variable(PC_LIBELF_INCLUDE_DIRS libelf includedir)
  endif()

  set(LibELF_INCLUDE_DIRS
      ${PC_LIBELF_INCLUDE_DIRS}
      CACHE PATH "")
  set(LibELF_LIBRARIES
      ${PC_LIBELF_LINK_LIBRARIES}
      CACHE PATH "")
  set(LibELF_VERSION
      ${PC_LIBELF_VERSION}
      CACHE STRING "")
else()
  find_path(
    LibELF_INCLUDE_DIRS
    NAMES libelf.h
    PATH_SUFFIXES elfutils ${_find_path_args})

  find_library(
    LibELF_LIBRARIES
    NAMES libelf elf
    PATH_SUFFIXES elfutils ${_find_path_args})

  macro(_check_libelf_version _file)
    file(STRINGS ${_file} _version_line REGEX "^#define _ELFUTILS_VERSION[ \t]+[0-9]+")
    string(REGEX MATCH "[0-9]+" _version "${_version_line}")
    if(NOT "x${_version}" STREQUAL "x")
      set(LibELF_VERSION "0.${_version}")
    endif()
    unset(_version_line)
    unset(_version)
  endmacro()

  if(EXISTS "${LibELF_INCLUDE_DIRS}/version.h")
    _check_libelf_version("${LibELF_INCLUDE_DIRS}/version.h")
  elseif(EXISTS "${LibELF_INCLUDE_DIRS}/elfutils/version.h")
    _check_libelf_version("${LibELF_INCLUDE_DIRS}/elfutils/version.h")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibELF
  FOUND_VAR LibELF_FOUND
  REQUIRED_VARS LibELF_LIBRARIES LibELF_INCLUDE_DIRS
  VERSION_VAR LibELF_VERSION)

if(LibELF_FOUND)
  mark_as_advanced(LibELF_INCLUDE_DIRS)
  mark_as_advanced(LibELF_LIBRARIES)
  mark_as_advanced(LibELF_VERSION)

  if(NOT TARGET LibELF::LibELF)
    add_library(LibELF::LibELF UNKNOWN IMPORTED)
    set_target_properties(
      LibELF::LibELF
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LibELF_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${LibELF_LIBRARIES}")
  endif()
endif()

unset(_find_path_args)
