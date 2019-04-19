#========================================================================================
# FindLibElf.cmake
#
# Find libelf include dirs and libraries
#
#		----------------------------------------
#
# Use this module by invoking find_package with the form::
#
#  find_package(LibElf
#    [version] [EXACT]      # Minimum or EXACT version e.g. 0.173
#    [REQUIRED]             # Fail with error if libelf is not found
#  )
#
# This module reads hints about search locations from variables::
#
#	LIBELF_ROOT			- Base directory the of libelf installation
#	LIBELF_INCLUDEDIR	- Hint directory that contains the libelf headers files
#	LIBELF_LIBRARYDIR	- Hint directory that contains the libelf library files
#
# and saves search results persistently in CMake cache entries::
#
#	LibElf_FOUND			- True if headers and requested libraries were found
#	LIBELF_INCLUDE_DIRS 	- libelf include directories
#	LIBELF_LIBRARY_DIRS		- Link directories for libelf libraries
#	LIBELF_LIBRARIES		- libelf library files
#
#
# Based on the version by Bernhard Walle <bernhard.walle@gmx.de> Copyright (c) 2008
#
#========================================================================================

include(DyninstSystemPaths)

# Non-standard subdirectories to search
set(_path_suffixes libelf libelfls elfutils)

find_path(LIBELF_INCLUDE_DIR
          NAMES libelf.h
          HINTS ${LIBELF_ROOT}/include ${LIBELF_ROOT} ${LIBELF_INCLUDEDIR}
          PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
          PATH_SUFFIXES ${_path_suffixes}
          DOC "libelf include directories")

find_library(LIBELF_LIBRARIES
             NAMES libelf.so.1 libelf.so
             HINTS ${LIBELF_ROOT}/lib ${LIBELF_ROOT} ${LIBELF_LIBRARYDIR}
             PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
             PATH_SUFFIXES ${_path_suffixes})

# Find the library with the highest version
set(_max_ver 0.0)
set(_max_ver_lib)
foreach(l ${LIBELF_LIBRARIES})
  get_filename_component(_elf_realpath ${LIBELF_LIBRARIES} REALPATH)
  string(REGEX MATCH
               "libelf\\-(.+)\\.so\\.*$"
               res
               ${_elf_realpath})

  # The library version number is stored in CMAKE_MATCH_1
  set(_cur_ver ${CMAKE_MATCH_1})

  if(${_cur_ver} VERSION_GREATER ${_max_ver})
    set(_max_ver ${_cur_ver})
    set(_max_ver_lib ${l})
  endif()
endforeach()

# Set the exported variables to the best match
set(LIBELF_LIBRARIES ${_max_ver_lib})
set(LibElf_VERSION ${_max_ver})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibElf
                                  FOUND_VAR
                                  LibElf_FOUND
                                  REQUIRED_VARS
                                  LIBELF_LIBRARIES
                                  LIBELF_INCLUDE_DIR
                                  VERSION_VAR
                                  LibElf_VERSION)
