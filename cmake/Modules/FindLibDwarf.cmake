#===================================================================================
# FindLibDwarf.cmake
#
# Find libdw include dirs and libraries
#
#		----------------------------------------
#
# Use this module by invoking find_package with the form::
#
#  find_package(LibDwarf
#    [version] [EXACT]      # Minimum or EXACT version e.g. 0.173
#    [REQUIRED]             # Fail with error if libdw is not found
#  )
#
# This module reads hints about search locations from variables::
#
#	LIBDWARF_ROOT			- Base directory the of libdw installation
#	LIBDWARF_INCLUDEDIR		- Hint directory that contains the libdw headers files
#	LIBDWARF_LIBRARYDIR		- Hint directory that contains the libdw library files
#
# and saves search results persistently in CMake cache entries::
#
#	LibDwarf_FOUND			- True if headers and requested libraries were found
#	LIBDWARF_INCLUDE_DIRS 	- libdw include directories
#	LIBDWARF_LIBRARY_DIRS	- Link directories for libdw libraries
#	LIBDWARF_LIBRARIES		- libdw library files
#
#===================================================================================

include(DyninstSystemPaths)

# Non-standard subdirectories to search
set(_path_suffixes libdw libdwarf elfutils)

find_path(LIBDWARF_INCLUDE_DIR
          NAMES libdw.h
          HINTS ${LIBDWARF_ROOT}/include ${LIBDWARF_ROOT} ${LIBDWARF_INCLUDEDIR}
          PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
          PATH_SUFFIXES ${_path_suffixes}
          DOC "libdw include directories")

find_library(LIBDWARF_LIBRARIES
             NAMES libdw.so.1 libdw.so
             HINTS ${LIBDWARF_ROOT}/lib ${LIBDWARF_ROOT} ${LIBDWARF_LIBRARYDIR}
             PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
             PATH_SUFFIXES ${_path_suffixes})

# Find the library with the highest version
set(_max_ver 0.0)
set(_max_ver_lib)
foreach(l ${LIBDWARF_LIBRARIES})
  get_filename_component(_dw_realpath ${LIBDWARF_LIBRARIES} REALPATH)
  string(REGEX MATCH
               "libdw\\-(.+)\\.so\\.*$"
               res
               ${_dw_realpath})

  # The library version number is stored in CMAKE_MATCH_1
  set(_cur_ver ${CMAKE_MATCH_1})

  if(${_cur_ver} VERSION_GREATER ${_max_ver})
    set(_max_ver ${_cur_ver})
    set(_max_ver_lib ${l})
  endif()
endforeach()

# Set the exported variables to the best match
set(LIBDWARF_LIBRARIES ${_max_ver_lib})
set(LIBDWARF_VERSION ${_max_ver})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibDwarf
                                  FOUND_VAR
                                  LibDwarf_FOUND
                                  REQUIRED_VARS
                                  LIBDWARF_LIBRARIES
                                  LIBDWARF_INCLUDE_DIR
                                  VERSION_VAR
                                  LIBDWARF_VERSION)

#mark_as_advanced(LIBDW_INCLUDE_DIR DWARF_INCLUDE_DIR)
#mark_as_advanced(LIBDWARF_INCLUDE_DIRS LIBDWARF_LIBRARIES)
