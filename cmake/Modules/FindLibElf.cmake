# - Try to find libelf
# Once done this will define
#
#  LIBELF_FOUND - system has libelf
#  LIBELF_INCLUDE_DIRS - the libelf include directory
#  LIBELF_LIBRARIES - Link these to use libelf
#  LIBELF_DEFINITIONS - Compiler switches required for using libelf
#
#  Copyright (c) 2008 Bernhard Walle <bernhard.walle@gmx.de>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (LIBELF_LIBRARIES AND LIBELF_INCLUDE_DIRS)
  set (LibElf_FIND_QUIETLY TRUE)
endif (LIBELF_LIBRARIES AND LIBELF_INCLUDE_DIRS)

find_path (LIBELF_INCLUDE_DIR
    NAMES
      libelf.h
    HINTS
      ${LIBELF_INCLUDE_DIRS}
    PATHS
      /usr/include
      /usr/include/libelf
      /usr/include/x86_64-linux-gnu
      /usr/local/include
      /usr/local/include/libelfls 
      /opt/local/include
      /opt/local/include/libelf
      /sw/include
      /sw/include/libelf
      ENV CPATH)

find_library (LIBELF_LIBRARIES
    NAMES
      libelf.so
    HINTS
      ${LIBELF_LIBRARIES}
    PATHS
      /usr/lib
      /usr/lib64
      /usr/local/lib
      /usr/local/lib64
      /usr/lib/x86_64-linux-gnu
      /opt/local/lib
      /opt/local/lib64
      /sw/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)

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
