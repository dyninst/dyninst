# * Try to find thread_db Once done this will define
#
# THREAD_DB_FOUND - system has thread_db THREAD_DB_INCLUDE_DIRS - the thread_db include
# directory THREAD_DB_LIBRARIES - Link these to use thread_db THREAD_DB_DEFINITIONS -
# Compiler switches required for using thread_db
#

if(THREAD_DB_LIBRARIES AND THREAD_DB_INCLUDE_DIRS)
    set(Thread_Db_FIND_QUIETLY TRUE)
endif(THREAD_DB_LIBRARIES AND THREAD_DB_INCLUDE_DIRS)

find_path(
    THREAD_DB_INCLUDE_DIR
    NAMES thread_db.h
    HINTS ${THREAD_DB_INCLUDE_DIRS}
    PATHS /usr/include
          /usr/include/thread_db
          /usr/local/include
          /opt/local/include
          /sw/include
          ENV
          CPATH) # PATH and INCLUDE will also work

find_library(
    THREAD_DB_LIBRARIES
    NAMES thread_db
    HINTS ${THREAD_DB_LIBRARIES}
    PATHS /usr/lib
          /usr/lib64
          /usr/local/lib
          /usr/local/lib64
          /opt/local/lib
          /opt/local/lib64
          /sw/lib
          ENV
          LIBRARY_PATH # PATH and LIB will also work
          ENV
          LD_LIBRARY_PATH)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set THREAD_DB_FOUND to TRUE if all listed
# variables are TRUE
find_package_handle_standard_args(Thread_DB DEFAULT_MSG THREAD_DB_LIBRARIES
                                  THREAD_DB_INCLUDE_DIR)

if(Thread_DB_FOUND)
    add_library(Thread_DB::Thread_DB INTERFACE IMPORTED)
    target_include_directories(Thread_DB::Thread_DB INTERFACE ${THREAD_DB_INCLUDE_DIR})
    target_link_libraries(Thread_DB::Thread_DB INTERFACE ${THREAD_DB_LIBRARIES})
endif()
