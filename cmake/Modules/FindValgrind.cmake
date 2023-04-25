#[=======================================================================[.rst:
FindLibValgrind
---------------

Find valgrind, a dynamic binary instrumentation framework.

Variables that affect this module

``Valgrind_NO_SYSTEM_PATHS``
  If `True`, no system paths are searched.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``Valgrind::Valgrind``
  The valgrind library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Valgrind_INCLUDE_DIRS``
  where to find valgrind.h, etc.
``Valgrind_FOUND``
  If false, do not try to use valgrind.
``Valgrind_VERSION``
  the version of the valgrind library found

#]=======================================================================]
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(Valgrind_NO_SYSTEM_PATHS)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

if(NOT Valgrind_NO_SYSTEM_PATHS)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    if(NOT "x${Valgrind_FIND_VERSION}" STREQUAL "x")
      set(_version ">=${Valgrind_FIND_VERSION}")
    endif()
    if(Valgrind_FIND_QUIETLY)
      set(_quiet "QUIET")
    endif()

    pkg_check_modules(PC_VALGRIND ${_quiet} "valgrind${_version}")
    unset(_version)
    unset(_quiet)
  endif()
endif()

if(PC_VALGRIND_FOUND)
  # FindPkgConfig sometimes gets the include dir wrong
  if("x${PC_VALGRIND_INCLUDE_DIRS}" STREQUAL "x")
    pkg_get_variable(PC_VALGRIND_INCLUDE_DIRS valgrind includedir)
  endif()

  set(Valgrind_INCLUDE_DIRS
      ${PC_VALGRIND_INCLUDE_DIRS}
      CACHE PATH "")
  set(Valgrind_VERSION
      ${PC_VALGRIND_VERSION}
      CACHE STRING "")
else()
  find_path(
    Valgrind_INCLUDE_DIRS
    NAMES valgrind.h
    PATH_SUFFIXES valgrind ${_find_path_args})

  macro(_check_valgrind_version _file)
    file(STRINGS ${_file} _version_line REGEX "^#define __VALGRIND_MAJOR__[ \t]+[0-9]+")
    string(REGEX MATCH "[0-9]+" _major "${_version_line}")
    file(STRINGS ${_file} _version_line REGEX "^#define __VALGRIND_MINOR__[ \t]+[0-9]+")
    string(REGEX MATCH "[0-9]+" _minor "${_version_line}")
    set(Valgrind_VERSION "${_major}.${_minor}")
    unset(_version_line)
    unset(_major)
    unset(_minor)
  endmacro()

  if(EXISTS "${Valgrind_INCLUDE_DIRS}/valgrind.h")
    _check_valgrind_version("${Valgrind_INCLUDE_DIRS}/valgrind.h")
  elseif(EXISTS "${Valgrind_INCLUDE_DIRS}/valgrind/valgrind.h")
    _check_valgrind_version("${Valgrind_INCLUDE_DIRS}/valgrind/valgrind.h")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Valgrind
  FOUND_VAR Valgrind_FOUND
  REQUIRED_VARS Valgrind_INCLUDE_DIRS
  VERSION_VAR Valgrind_VERSION)

mark_as_advanced(Valgrind_INCLUDE_DIRS)

unset(_find_path_args)
