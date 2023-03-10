#[=======================================================================[.rst:
FindElfutils
------------

Find elfutils, a collection of utilities and libraries to read, create
and modify ELF binary files, find and handle DWARF debug data,
symbols, thread state and stacktraces for processes and core files
on GNU/Linux.

Variables that affect this module

``ElfUtils_NO_SYSTEM_PATHS``
  If `True`, no system paths are searched.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``Elfutils::Elfutils``
  The elfutils library, if found.

This module will set the following variables in your project:

``Elfutils_INCLUDE_DIRS``
  where to find elfutils headers
``Elfutils_LIBRARIES``
  the libraries to link against to use elfutils.
``Elfutils_FOUND``
  If false, do not try to use elfutils.
``Elfutils_VERSION``
  the version of the elfutils library found

Support for libdebuginfod can be added by specifying it in ``COMPONENTS``.

.. code-block:: cmake

   find_package(Elfutils 0.186 EXACT REQUIRED COMPONENTS debuginfod)

#]=======================================================================]
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(${Elfutils_FIND_REQUIRED})
  set(_required "REQUIRED")
endif()

if(${Elfutils_FIND_QUIETLY})
  set(_quiet "QUIET")
endif()

if(${Elfutils_FIND_EXACT})
  set(_exact "EXACT")
endif()

# Propagate ElfUtils_NO_SYSTEM_PATHS
foreach(_n "LibELF" "LibDW" "LibDebuginfod")
  set(${_n}_NO_SYSTEM_PATHS ${ElfUtils_NO_SYSTEM_PATHS})
  mark_as_advanced(${_n}_NO_SYSTEM_PATHS)

  # Force the search directory
  if(ElfUtils_NO_SYSTEM_PATHS)
    set(${_n}_ROOT ${ElfUtils_ROOT_DIR})
    mark_as_advanced(${_n}_ROOT)
  endif()
endforeach()

find_package(LibELF ${Elfutils_FIND_VERSION} ${_exact} ${_required} ${_quiet})
find_package(LibDW ${Elfutils_FIND_VERSION} ${_exact} ${_required} ${_quiet})

if(NOT "x${Elfutils_FIND_COMPONENTS}" STREQUAL "x")
  string(TOUPPER ${Elfutils_FIND_COMPONENTS} _tmp)
  if(NOT ${_tmp} STREQUAL "DEBUGINFOD")
    message(FATAL "Unknown component: '${Elfutils_FIND_COMPONENTS}'")
  endif()
  find_package(LibDebuginfod ${Elfutils_FIND_VERSION} ${_exact} ${_required} ${_quiet})
  set(_need_debuginfod TRUE)
  unset(_tmp)
endif()

# Ensure that each component has the same version number
set(_versions ${LibDW_VERSION} ${LibELF_VERSION} ${LibDebuginfod_VERSION})
list(REMOVE_DUPLICATES _versions)
list(LENGTH _versions _len)
if(${_len} GREATER 1)
  message(FATAL_ERROR "Elfutils: conflicting versions found: (${_versions})")
endif()
unset(_len)

set(Elfutils_VERSION ${_versions})
unset(_versions)

include(FindPackageHandleStandardArgs)
if(${_need_debuginfod})
  find_package_handle_standard_args(
    Elfutils
    FOUND_VAR Elfutils_FOUND
    REQUIRED_VARS LibDW_INCLUDE_DIRS LibDW_LIBRARIES LibELF_INCLUDE_DIRS LibELF_LIBRARIES
                  LibDebuginfod_INCLUDE_DIRS LibDebuginfod_LIBRARIES
    VERSION_VAR Elfutils_VERSION)
else()
  find_package_handle_standard_args(
    Elfutils
    FOUND_VAR Elfutils_FOUND
    REQUIRED_VARS LibDW_INCLUDE_DIRS LibDW_LIBRARIES LibELF_INCLUDE_DIRS LibELF_LIBRARIES
    VERSION_VAR Elfutils_VERSION)
endif()

if(Elfutils_FOUND)
  set(Elfutils_INCLUDE_DIRS
      ${LibDW_INCLUDE_DIRS} ${LibELF_INCLUDE_DIRS} ${LibDebuginfod_INCLUDE_DIRS}
      CACHE PATH "")
  mark_as_advanced(Elfutils_INCLUDE_DIRS)

  set(Elfutils_LIBRARIES
      ${LibDW_LIBRARIES} ${LibELF_LIBRARIES} ${LibDebuginfod_LIBRARIES}
      CACHE PATH "")
  mark_as_advanced(Elfutils_LIBRARIES)

  mark_as_advanced(Elfutils_VERSION)

  if(NOT TARGET Elfutils::Elfutils)
    add_library(Elfutils::Elfutils INTERFACE IMPORTED)
    target_link_libraries(Elfutils::Elfutils INTERFACE LibELF::LibELF LibDW::LibDW)
    if(${_need_debuginfod})
      target_link_libraries(Elfutils::Elfutils INTERFACE LibDebuginfod::LibDebuginfod)
    endif()
  endif()
endif()

unset(_exact)
unset(_quiet)
unset(_required)
unset(_need_debuginfod)
