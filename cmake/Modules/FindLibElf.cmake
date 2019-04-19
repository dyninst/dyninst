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

# Enforce required version, if enabled
if(LibElf_FIND_VERSION AND LIBELF_LIBRARIES)
	get_filename_component(LibElf_realpath ${LIBELF_LIBRARIES} REALPATH)
	string(REGEX MATCH "libelf\\-(.+)\\.so$" res ${LibElf_realpath})
	
	# The library version number is stored in CMAKE_MATCH_1
	set(LibELf_version ${CMAKE_MATCH_1})
	
	if(NOT res OR NOT ${LibELf_version})
		message(WARNING "Found libelf library '${LibElf_realpath}', but does not match known name format")
		set(LibElf_FOUND FALSE)
		set(LIBELF_FOUND FALSE)
		return()
	endif()
	
	if(${LibELf_version} VERSION_LESS ${LibElf_FIND_VERSION})
		message(WARNING "Found libelf version '${LibELf_version}', but required version '${LibElf_FIND_VERSION}'")
		set(LibElf_FOUND FALSE)
		set(LIBELF_FOUND FALSE)
		return()
	endif()
	set(LIBELF_FOUND TRUE)
	set(LibElf_FOUND TRUE)
	message(STATUS "Found libelf '${LibELf_version}' in '${LibElf_realpath}'")
	return()
else()
	include (FindPackageHandleStandardArgs)
	
	#handle the QUIETLY and REQUIRED arguments and set LIBELF_FOUND to TRUE if all listed variables are TRUE
	find_package_handle_standard_args(LibElf LIBELF_LIBRARIES LIBELF_INCLUDE_DIR)
endif()
