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


if (LIBDWARF_LIBRARIES AND LIBDWARF_INCLUDE_DIRS)
  set (LibDwarf_FIND_QUIETLY TRUE)
endif (LIBDWARF_LIBRARIES AND LIBDWARF_INCLUDE_DIRS)

find_path (LIBDWARF_INCLUDE_DIR
  NAMES
  elfutils/libdw.h
  HINTS
  ${LIBDWARF_INCLUDE_DIRS}
  PATHS
  /usr/include
  /usr/local/include
  /opt/local/include
  /sw/include
  ENV CPATH) # PATH and INCLUDE will also work

find_library (LIBDWARF_LIBRARIES
  NAMES
  dw
  HINTS
  ${LIBDWARF_LIBRARIES}
  PATHS
  /usr/lib
  /usr/lib64
  /usr/local/lib
  /usr/local/lib64
  /opt/local/lib
  /opt/local/lib64
  /sw/lib
  ENV LIBRARY_PATH   # PATH and LIB will also work
  ENV LD_LIBRARY_PATH)
include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBDWARF_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibDwarf DEFAULT_MSG
  LIBDWARF_LIBRARIES
  LIBDWARF_INCLUDE_DIR)

#mark_as_advanced(LIBDW_INCLUDE_DIR DWARF_INCLUDE_DIR)
#mark_as_advanced(LIBDWARF_INCLUDE_DIRS LIBDWARF_LIBRARIES)
