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
if(NOT LibElf_FOUND OR NOT LibDwarf_FOUND)
  message(STATUS "Attempting to build elfutils as external project")
  include(ExternalProject)
  externalproject_add(
    ElfUtils # was LibElf
    PREFIX ${CMAKE_BINARY_DIR}/elfutils
    URL https://sourceware.org/elfutils/ftp/elfutils-latest.tar.bz2
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND
      CFLAGS=-g
      <SOURCE_DIR>/configure
      --enable-install-elfh
      --enable-shared
      --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
  )
  set(LIBELF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/elfutils/include)
  set(LIBELF_LIBRARIES ${CMAKE_BINARY_DIR}/elfutils/lib/libelf.so)
  set(LIBDWARF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/elfutils/include)
  set(LIBDWARF_LIBRARIES ${CMAKE_BINARY_DIR}/elfutils/lib/libdw.so)
  set(SHOULD_INSTALL_LIBELF 1)
else()
  set(SHOULD_INSTALL_LIBELF 0)
endif()
