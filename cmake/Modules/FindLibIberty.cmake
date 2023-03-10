#[=======================================================================[.rst:
FindLibIberty
-------------

Find libiberty, a collection of subroutines used by various GNU programs.

Variables that affect this module

``LibIberty_NO_SYSTEM_PATHS``
  If `True`, no system paths are searched.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``LibIberty::LibIberty``
  The libiberty library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``LibIberty_INCLUDE_DIRS``
  where to find libiberty.h, etc.
``LibIberty_LIBRARIES``
  the libraries to link against to use libiberty.
``LibIberty_FOUND``
  If false, do not try to use libiberty.

 LibIberty does not have its own version number or release schedule.
 See https://gcc.gnu.org/onlinedocs/libiberty/Using.html#Using for details.

#]=======================================================================]
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(LibIberty_NO_SYSTEM_PATHS)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

find_path(
  LibIberty_INCLUDE_DIRS
  NAMES libiberty.h
  PATH_SUFFIXES libiberty ${_find_path_args})
mark_as_advanced(LibIberty_INCLUDE_DIRS)

find_library(
  LibIberty_LIBRARIES
  NAMES libiberty iberty
  PATH_SUFFIXES libiberty ${_find_path_args})
mark_as_advanced(LibIberty_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibIberty
  FOUND_VAR LibIberty_FOUND
  REQUIRED_VARS LibIberty_LIBRARIES LibIberty_INCLUDE_DIRS)

if(LibIberty_FOUND)
  if(NOT TARGET LibIberty::LibIberty)
    add_library(LibIberty::LibIberty UNKNOWN IMPORTED)
    set_target_properties(
      LibIberty::LibIberty
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LibIberty_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${LibIberty_LIBRARIES}")
  endif()
endif()

unset(_find_path_args)
