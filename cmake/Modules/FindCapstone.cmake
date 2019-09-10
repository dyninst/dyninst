#===================================================================================
# FindCapstone.cmake
#
# Find Capstone include dirs and libraries
#
#		----------------------------------------
#
# Use this module by invoking find_package with the form
#
#  find_package(Capstone
#    [version] [EXACT]      # Minimum or EXACT version e.g. 5.0
#    [REQUIRED]             # Fail with error if Capstone is not found
#  )
#
# This module reads hints about search locations from variables::
#
#	Capstone_ROOT_DIR		     - Base directory the of capstone installation
#	Capstone_INCLUDEDIR		     - Hint directory that contains the capstone headers files
#	Capstone_LIBRARYDIR		     - Hint directory that contains the capstone library files
#   Capstone_USE_STATIC_RUNTIME  - Use static runtime
#
# and saves search results persistently in CMake cache entries::
#
#	Capstone_FOUND			- True if headers and requested libraries were found
#	Capstone_INCLUDE_DIRS 	- capstone include directories
#	Capstone_LIBRARY_DIRS	- Link directories for capstone libraries
#	Capstone_LIBRARIES		- capstone library files
#
#===================================================================================

include(DyninstSystemPaths)

set(_cs_lib_name "libcapstone.so")
if(Capstone_USE_STATIC_RUNTIME)
  set(_cs_lib_name "libcapstone.a")
endif()

find_path(Capstone_INCLUDE_DIR
          NAMES capstone.h
          HINTS ${Capstone_ROOT_DIR}/include ${Capstone_ROOT_DIR}
                ${Capstone_INCLUDEDIR}
          PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
          PATH_SUFFIXES capstone
          DOC "capstone include directories")

find_library(Capstone_LIBRARIES
             NAMES ${_cs_lib_name}
             HINTS ${Capstone_ROOT_DIR} ${Capstone_LIBRARYDIR}
                   ${Capstone_ROOT_DIR}/lib ${Capstone_ROOT_DIR}/lib64
             PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
             PATH_SUFFIXES capstone)

# Determine the version
foreach(v IN ITEMS major minor)
  string(TOUPPER ${v} v_upper)
  file(STRINGS "${Capstone_INCLUDE_DIR}/capstone.h" _tmp_${v}
       REGEX "#define CS_API_${v_upper}")
  if("${_tmp_${v}}" MATCHES "#define CS_API_${v_upper} ([0-9]+)")
    set(_capstone_${v}_ver ${CMAKE_MATCH_1})
  else()
    message(FATAL_ERROR "Unable to determine ${v} version of Capstone")
  endif()
  unset(_tmp_${v})
endforeach()

set(Capstone_VERSION "${_capstone_major_ver}.${_capstone_minor_ver}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Capstone
                                  FOUND_VAR
                                  Capstone_FOUND
                                  REQUIRED_VARS
                                  Capstone_LIBRARIES
                                  Capstone_INCLUDE_DIR
                                  VERSION_VAR
                                  Capstone_VERSION)

# Export cache variables
if(Capstone_FOUND)
  set(Capstone_INCLUDE_DIRS
      ${Capstone_INCLUDE_DIR} 		# for #include "capstone.h"
      ${Capstone_INCLUDE_DIR}/.. 	# for #include "capstone/capstone.h"
      CACHE PATH "Capstone include directories")
  get_filename_component(_cs_dir ${Capstone_LIBRARIES} DIRECTORY)
  set(Capstone_LIBRARY_DIRS ${_cs_dir}
      CACHE PATH "Link directories for capstone libraries")

  if(Capstone_USE_STATIC_RUNTIME)
    add_library(capstone STATIC IMPORTED)
  else()
    add_library(capstone SHARED IMPORTED)
  endif()
endif()

unset(_cs_lib_name)
