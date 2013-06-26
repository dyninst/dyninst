# - Try to find DIASDK
# Once done this will define
#
#  DIASDK_FOUND - system has DIASDK
#  DIASDK_INCLUDE_DIRS - the DIASDK include directory
#  DIASDK_LIBRARIES - Link these to use DIASDK
#  DIASDK_DEFINITIONS - Compiler switches required for using DIASDK
#
if(WIN32)
if (DIASDK_INCLUDE_DIRS)
  set (DIASDK_FIND_QUIETLY TRUE)
endif (DIASDK_INCLUDE_DIRS)

find_path (DIASDK_INCLUDE_DIR
    NAMES
      cvconst.h
    HINTS
      ${DIASDK_INCLUDE_DIRS}
    PATHS
      ENV CPATH) # PATH and INCLUDE will also work

#find_library (DIASDK_LIBRARIES
#    NAMES
#      dbghlp
#    HINTS
#      ${DIASDK_LIBRARIES}
#    PATHS
#      ENV LIBRARY_PATH   # PATH and LIB will also work
#      ENV LD_LIBRARY_PATH)
include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set DIASDK_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DIASDK DEFAULT_MSG
    DIASDK_INCLUDE_DIR)
endif()

