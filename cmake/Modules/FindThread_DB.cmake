#[=======================================================================[.rst:
FindThread_DB
-------------

Find thread_db, the debugger interface for the NPTL library.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``Thread_DB::Thread_DB``
  The threaddb library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Thread_DB_INCLUDE_DIRS``
  where to find thread_db.h, etc.
``Thread_DB_LIBRARIES``
  the libraries to link against to use thread_db.
``Thread_DB_FOUND``
  If false, do not try to use thread_db.

 Thread_DB does not have its own version number or release schedule.
 See https://sourceware.org/gdb/current/onlinedocs/gdb/Threads.html for details.

#]=======================================================================]
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

find_path(Thread_DB_INCLUDE_DIRS NAMES thread_db.h)

find_library(Thread_DB_LIBRARIES NAMES thread_db)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Thread_DB
  FOUND_VAR Thread_DB_FOUND
  REQUIRED_VARS Thread_DB_LIBRARIES Thread_DB_INCLUDE_DIRS)

if(Thread_DB_FOUND)
  if(NOT TARGET Thread_DB::Thread_DB)
    add_library(Thread_DB::Thread_DB UNKNOWN IMPORTED)
    set_target_properties(
      Thread_DB::Thread_DB
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Thread_DB_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${Thread_DB_LIBRARIES}")
  endif()

  mark_as_advanced(Thread_DB_INCLUDE_DIRS)
  mark_as_advanced(Thread_DB_LIBRARIES)
endif()
