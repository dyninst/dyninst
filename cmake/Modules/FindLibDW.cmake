#[=======================================================================[.rst:
FindLibDW
---------

Find libdw, the elfutils library for DWARF data and ELF file or process inspection.

Variables that affect this module

``LibDW_NO_SYSTEM_PATHS``
  If `True`, no system paths are searched.

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

if(LibDW_NO_SYSTEM_PATHS)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

# There is no way to tell pkg-config to ignore directories, so disable it
if(NOT LibDW_NO_SYSTEM_PATHS)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    if(NOT "x${LibDW_FIND_VERSION}" STREQUAL "x")
      set(_version ">=${LibDW_FIND_VERSION}")
    endif()
    if(LibDW_FIND_QUIETLY)
      set(_quiet "QUIET")
    endif()

    pkg_check_modules(PC_LIBDW ${_quiet} "libdw${_version}")
    unset(_version)
    unset(_quiet)
  endif()
endif()

if(PC_LIBDW_FOUND)
  # FindPkgConfig sometimes gets the include dir wrong
  if("x${PC_LIBDW_INCLUDE_DIRS}" STREQUAL "x")
    pkg_get_variable(PC_LIBDW_INCLUDE_DIRS libdw includedir)
  endif()

  set(LibDW_INCLUDE_DIRS
      ${PC_LIBDW_INCLUDE_DIRS}
      CACHE PATH "")
  set(LibDW_LIBRARIES
      ${PC_LIBDW_LINK_LIBRARIES}
      CACHE PATH "")
  set(LibDW_VERSION
      ${PC_LIBDW_VERSION}
      CACHE STRING "")
else()
  find_path(
    LibDW_INCLUDE_DIRS
    NAMES libdw.h
    PATH_SUFFIXES elfutils ${_find_path_args})

  find_library(
    LibDW_LIBRARIES
    NAMES libdw dw
    PATH_SUFFIXES elfutils ${_find_path_args})

  if(EXISTS "${LibDW_INCLUDE_DIRS}/version.h")
    file(STRINGS "${LibDW_INCLUDE_DIRS}/version.h" _version_line
         REGEX "^#define _ELFUTILS_VERSION[ \t]+[0-9]+")
    string(REGEX MATCH "[0-9]+" _version "${_version_line}")
    if(NOT "x${_version}" STREQUAL "x")
      set(LibDW_VERSION "0.${_version}")
    endif()
    unset(_version_line)
    unset(_version)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibDW
  FOUND_VAR LibDW_FOUND
  REQUIRED_VARS LibDW_LIBRARIES LibDW_INCLUDE_DIRS
  VERSION_VAR LibDW_VERSION)

if(LibDW_FOUND)
  mark_as_advanced(LibDW_INCLUDE_DIRS)
  mark_as_advanced(LibDW_LIBRARIES)
  mark_as_advanced(LibDW_VERSION)

  # Some platforms explicitly list libelf as a dependency, so separate it out
  list(LENGTH LibDW_LIBRARIES _cnt)
  if(${_cnt} GREATER 1)
    foreach(_l ${LibDW_LIBRARIES})
      if(${_l} MATCHES "libdw")
        set(_libdw ${_l})
      else()
        list(APPEND _link_libs ${_l})
      endif()
    endforeach()
  endif()
  unset(_cnt)

  if(NOT TARGET LibDW::LibDW)
    add_library(LibDW::LibDW UNKNOWN IMPORTED)
    set_target_properties(LibDW::LibDW PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                  "${LibDW_INCLUDE_DIRS}")

    if(NOT "x${_link_libs}" STREQUAL "x")
      set_target_properties(
        LibDW::LibDW PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                IMPORTED_LINK_DEPENDENT_LIBRARIES "${_link_libs}")
      set(LibDW_LIBRARIES ${_libdw})
      unset(_libdw)
      unset(_link_libs)
    endif()

    set_target_properties(LibDW::LibDW PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                                  IMPORTED_LOCATION "${LibDW_LIBRARIES}")
  endif()
endif()

unset(_find_path_args)
