#====================================================================================================
# elfutils.cmake
#
# Configure elfutils for Dyninst
#
#		----------------------------------------
#
# Accepts the following CMake variables
#
#	ElfUtils_ROOT				- Base directory the of elfutils installation
#	ElfUtils_INCLUDEDIR			- Hint directory that contains the elfutils headers files
#	ElfUtils_LIBRARYDIR			- Hint directory that contains the elfutils library files
#	ElfUtils_MIN_VERSION		- Minimum acceptable version of elfutils
#
# Directly exports the following CMake variables
#
#	ElfUtils_ROOT				- Computed base directory the of elfutils installation
#	ElfUtils_INCLUDE_DIRS 		- elfutils include directories
#	ElfUtils_LIBRARY_DIRS		- Link directories for elfutils libraries
#	ElfUtils_LIBRARIES			- elfutils library files
#
# NOTE:
#	The exported ElfUtils_ROOT can be different from the input variable
#	in the case that it is determined to build elfutils from source. In such
#	a case, ElfUtils_ROOT will contain the directory of the from-source
#	installation.
#
#====================================================================================================
if(NOT UNIX)
  return()
endif()

# Minimum acceptable version of elfutils
set(_min_version 0.173)
set(ElfUtils_MIN_VERSION ${_min_version}
    CACHE STRING "Minimum acceptable elfutils version")
if(${ElfUtils_MIN_VERSION} VERSION_LESS ${_min_version})
  message(
    FATAL_ERROR
      "Requested version ${ElfUtils_MIN_VERSION} is less than minimum supported version (${_min_version})"
    )
endif()

# -------------- PATHS --------------------------------------------------------

# Base directory the of elfutils installation
set(ElfUtils_ROOT "/usr"
    CACHE PATH "Base directory the of elfutils installation")

# Hint directory that contains the elfutils headers files
set(ElfUtils_INCLUDEDIR "${ElfUtils_ROOT}/include"
    CACHE PATH "Hint directory that contains the elfutils headers files")

# Hint directory that contains the elfutils library files
set(ElfUtils_LIBRARYDIR "${ElfUtils_ROOT}/lib"
    CACHE PATH "Hint directory that contains the elfutils library files")

# libelf/dwarf-specific directory hints
foreach(l LIBELF LIBDWARF)
  foreach(d ROOT INCLUDEDIR LIBRARYDIR)
    set(${l}_${d} ${ElfUtils_${d}})
  endforeach()
endforeach()

# -------------- PACKAGES------------------------------------------------------

find_package(LibElf ${ElfUtils_MIN_VERSION})

# Don't search for libdw if we didn't find a suitable libelf
if(LibElf_FOUND)
  find_package(LibDwarf ${ElfUtils_MIN_VERSION})
endif()

# -------------- SOURCE BUILD -------------------------------------------------
if(LibElf_FOUND AND LibDwarf_FOUND)
  set(_eu_root ${ElfUtils_ROOT})
  set(_eu_inc_dirs ${LibElf_INCLUDE_DIRS} ${LibDwarf_INCLUDE_DIRS})
  set(_eu_lib_dirs ${LibElf_LIBRARY_DIRS} ${LibDwarf_LIBRARY_DIRS})
  set(_eu_libs ${LibElf_LIBRARIES} ${LibDwarf_LIBRARIES})
  add_library(ElfUtils SHARED IMPORTED)
else()
  message(
    STATUS
      "Attempting to build elfutils(${ElfUtils_MIN_VERSION}) as external project"
    )

  # When building from source, we need at least elfutils-0.176 in order to use
  # the --enable-install-elf option
  set(_min_src_vers 0.176)
  if("${ElfUtils_MIN_VERSION}" VERSION_LESS "${_min_src_vers}")
    message(
      STATUS
        "Requested elfutils-${ElfUtils_MIN_VERSION}, but installing elfutils-${_min_src_vers}"
      )
    set(ElfUtils_MIN_VERSION ${_min_src_vers}
        CACHE STRING "Minimum acceptable elfutils version"
        FORCE)
  endif()

  include(ExternalProject)
  externalproject_add(
    ElfUtils
    PREFIX ${CMAKE_BINARY_DIR}/elfutils
    URL https://sourceware.org/elfutils/ftp/${_min_src_vers}/elfutils-${_min_src_vers}.tar.bz2
    URL_MD5 077e4f49320cad82bf17a997068b1db9
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND
      CFLAGS=-g
      <SOURCE_DIR>/configure
      --enable-install-elfh
      --enable-shared
      --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND make install
    INSTALL_COMMAND ""
  )

  set(_eu_root ${CMAKE_INSTALL_PREFIX})
  set(_eu_inc_dirs ${CMAKE_INSTALL_PREFIX}/include
      ${CMAKE_INSTALL_PREFIX}/include/elfutils)
  set(_eu_lib_dirs ${CMAKE_INSTALL_PREFIX}/lib
      ${CMAKE_INSTALL_PREFIX}/lib/elfutils)
  set(_eu_libs ${_eu_root}/lib/libelf.so ${_eu_root}/lib/libdw.so)
endif()

# -------------- EXPORT VARIABLES ---------------------------------------------

set(ElfUtils_ROOT ${_eu_root}
    CACHE PATH "Base directory the of elfutils installation"
    FORCE)
set(ElfUtils_INCLUDE_DIRS ${_eu_inc_dirs}
    CACHE PATH "elfutils include directory"
    FORCE)
set(ElfUtils_LIBRARY_DIRS ${_eu_lib_dirs}
    CACHE PATH "elfutils library directory"
    FORCE)
set(ElfUtils_INCLUDE_DIR ${ElfUtils_INCLUDE_DIRS}
    CACHE PATH "elfutils include directory"
    FORCE)
set(ElfUtils_LIBRARIES ${_eu_libs}
    CACHE FILEPATH "elfutils library files"
    FORCE)

link_directories(${ElfUtils_LIBRARY_DIRS})
include_directories(${ElfUtils_INCLUDE_DIRS})

message(STATUS "ElfUtils includes: ${ElfUtils_INCLUDE_DIRS}")
message(STATUS "ElfUtils library dirs: ${ElfUtils_LIBRARY_DIRS}")
message(STATUS "ElfUtils libraries: ${ElfUtils_LIBRARIES}")
