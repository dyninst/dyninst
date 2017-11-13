# - Try to find cilkplus
# Once done this will define
#
#  CILK_FOUND - system has cilk
#  CILK_INCLUDE_DIRS - the cilk include directory
#  CILK_LIBRARIES - Link these to use cilk
#  CILK_DEFINITIONS - Compiler switches required for using cilk
#

set(CILK_DEFINITIONS -fopenmp)
#set(CILK_DEFINITIONS -fcilkplus)


#find_library (CILK_LIBRARIES
#        NAMES
#        cilkrts
##        HINTS
##        ${CILK_LIBRARIES}
#        PATHS
#        /usr/lib
#        /usr/lib64
#        /usr/local/lib
#        /usr/local/lib64
#        /opt/local/lib
#        /opt/local/lib64
#        /sw/lib
#        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
#        ENV LIBRARY_PATH   # PATH and LIB will also work
#        ENV LD_LIBRARY_PATH)
#include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set CILK_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(cilk DEFAULT_MSG
        CILK_DEFINITIONS)

