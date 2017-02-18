# - Try to find libdwarf
# Once done this will define
#
#  LIBDWARF_FOUND - system has libdwarf
#  LIBDWARF_INCLUDE_DIRS - the libdwarf include directory
#  LIBDWARF_LIBRARIES - Link these to use libdwarf
#  LIBDWARF_DEFINITIONS - Compiler switches required for using libdwarf
#

# Locate libelf library at first
#if (NOT LIBELF_FOUND)
#   find_package (LibElf REQUIRED)
#endif (NOT LIBELF_FOUND)

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
