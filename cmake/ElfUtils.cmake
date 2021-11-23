#======================================================================================
# elfutils.cmake
#
# Configure elfutils for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# ElfUtils_ROOT_DIR       - Base directory the of elfutils installation
# ElfUtils_INCLUDEDIR     - Hint directory that contains the elfutils headers files
# ElfUtils_LIBRARYDIR     - Hint directory that contains the elfutils library files
# ElfUtils_MIN_VERSION    - Minimum acceptable version of elfutils
#
# Directly exports the following CMake variables
#
# ElfUtils_ROOT_DIR       - Computed base directory the of elfutils installation
# ElfUtils_INCLUDE_DIRS   - elfutils include directories
# ElfUtils_LIBRARY_DIRS   - Link directories for elfutils libraries
# ElfUtils_LIBRARIES      - elfutils library files
#
# NOTE:
# The exported ElfUtils_ROOT_DIR can be different from the value provided by the user
# in the case that it is determined to build elfutils from source. In such a case,
# ElfUtils_ROOT_DIR will contain the directory of the from-source installation.
#
# See Modules/FindLibElf.cmake and Modules/FindLibDwarf.cmake for details
#
#======================================================================================

if(LibElf_FOUND AND LibDwarf_FOUND AND NOT ENABLE_DEBUGINFOD)
  return()
endif()

if(NOT UNIX)
  return()
endif()

# Minimum acceptable version of elfutils
# NB: We need >=0.186 because of NVIDIA line map extensions
set(_min_version 0.186)

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
set(ElfUtils_ROOT_DIR "/usr"
    CACHE PATH "Base directory the of elfutils installation")

# Hint directory that contains the elfutils headers files
set(ElfUtils_INCLUDEDIR "${ElfUtils_ROOT_DIR}/include"
    CACHE PATH "Hint directory that contains the elfutils headers files")

# Hint directory that contains the elfutils library files
set(ElfUtils_LIBRARYDIR "${ElfUtils_ROOT_DIR}/lib"
    CACHE PATH "Hint directory that contains the elfutils library files")

# libelf/dwarf-specific directory hints
foreach(l LibElf LibDwarf LibDebuginfod)
  foreach(d ROOT_DIR INCLUDEDIR LIBRARYDIR)
    set(${l}_${d} ${ElfUtils_${d}})
  endforeach()
endforeach()

# -------------- PACKAGES------------------------------------------------------

find_package(LibElf ${ElfUtils_MIN_VERSION})

# Don't search for libdw or libdebuginfod if we didn't find a suitable libelf
if(LibElf_FOUND)
  find_package(LibDwarf ${ElfUtils_MIN_VERSION})
  if (ENABLE_DEBUGINFOD)
    find_package(LibDebuginfod ${ElfUtils_MIN_VERSION} REQUIRED)
  endif()
endif()

# -------------- SOURCE BUILD -------------------------------------------------
if(LibElf_FOUND AND LibDwarf_FOUND AND (NOT ENABLE_DEBUGINFOD OR LibDebuginfod_FOUND))
  if(ENABLE_DEBUGINFOD AND LibDebuginfod_FOUND)
    set(_eu_root ${ElfUtils_ROOT_DIR})
    set(_eu_inc_dirs ${LibElf_INCLUDE_DIRS} ${LibDwarf_INCLUDE_DIRS} ${LibDebuginfod_INCLUDE_DIRS})
    set(_eu_lib_dirs ${LibElf_LIBRARY_DIRS} ${LibDwarf_LIBRARY_DIRS} ${LibDebuginfod_LIBRARY_DIRS})
    set(_eu_libs ${LibElf_LIBRARIES} ${LibDwarf_LIBRARIES} ${LibDebuginfod_LIBRARIES})
  else()
    set(_eu_root ${ElfUtils_ROOT_DIR})
    set(_eu_inc_dirs ${LibElf_INCLUDE_DIRS} ${LibDwarf_INCLUDE_DIRS})
    set(_eu_lib_dirs ${LibElf_LIBRARY_DIRS} ${LibDwarf_LIBRARY_DIRS})
    set(_eu_libs ${LibElf_LIBRARIES} ${LibDwarf_LIBRARIES})
  endif()
  add_library(ElfUtils SHARED IMPORTED)
elseif(NOT (LibElf_FOUND AND LibDwarf_FOUND) AND STERILE_BUILD)
  message(FATAL_ERROR "Elfutils not found and cannot be downloaded because build is sterile.")
else()
  # If we didn't find a suitable version on the system, then download one from the web
  # NB: When building from source, we need at least elfutils-0.176 in order to use
  #     the --enable-install-elf option
  set(_elfutils_download_version 0.176)

  # If the user specified a version newer than _elfutils_download_version, use that version.
  # NB: We know ElfUtils_MIN_VERSION is >= _min_version from earlier checks
  if(${ElfUtils_MIN_VERSION} VERSION_GREATER ${_elfutils_download_version})
    set(_elfutils_download_version ${ElfUtils_MIN_VERSION})
  endif()

  message(STATUS "${ElfUtils_ERROR_REASON}")
  message( STATUS "Attempting to build elfutils(${_elfutils_download_version}) as external project")
  
  if(NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR NOT (${CMAKE_C_COMPILER_ID} STREQUAL "GNU"))
    message(FATAL_ERROR "ElfUtils will only build with the GNU compiler")
  endif()

  include(ExternalProject)
  externalproject_add(
    ElfUtils
    PREFIX ${CMAKE_BINARY_DIR}/elfutils
    URL https://sourceware.org/elfutils/ftp/${_elfutils_download_version}/elfutils-${_elfutils_download_version}.tar.bz2
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND
      CFLAGS=-g
      CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
      <SOURCE_DIR>/configure
      --enable-install-elfh
      --prefix=${CMAKE_INSTALL_PREFIX}
      --disable-debuginfod
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

set(ElfUtils_ROOT_DIR ${_eu_root}
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
