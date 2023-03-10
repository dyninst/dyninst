#[=======================================================================[.rst:
FindLibDebuginfod
-----------------

Find libdebuginfod, the elfutils library to query debuginfo files from debuginfod servers.

Variables that affect this module

``LibDebuginfod_NO_SYSTEM_PATHS``
  If `True`, no system paths are searched.

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

if(LibDebuginfod_NO_SYSTEM_PATHS)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

# There is no way to tell pkg-config to ignore directories, so disable it
if(NOT LibDebuginfod_NO_SYSTEM_PATHS)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    if(NOT "x${LibDebuginfod_FIND_VERSION}" STREQUAL "x")
      set(_version ">=${LibDebuginfod_FIND_VERSION}")
    endif()
    if(LibDebuginfod_FIND_QUIETLY)
      set(_quiet "QUIET")
    endif()

    pkg_check_modules(PC_LIBDEBUGINFOD ${_quiet} "libdebuginfod${_version}")
    unset(_version)
    unset(_quiet)
  endif()
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
    PATH_SUFFIXES elfutils ${_find_path_args})

  find_library(
    LibDebuginfod_LIBRARIES
    NAMES libdebuginfod debuginfod
    PATH_SUFFIXES elfutils ${_find_path_args})

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
  mark_as_advanced(LibDebuginfod_VERSION)

  if(NOT TARGET LibDebuginfod::LibDebuginfod)
    add_library(LibDebuginfod::LibDebuginfod UNKNOWN IMPORTED)
    set_target_properties(
      LibDebuginfod::LibDebuginfod
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LibDebuginfod_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${LibDebuginfod_LIBRARIES}")
  endif()
endif()

unset(_find_path_args)
