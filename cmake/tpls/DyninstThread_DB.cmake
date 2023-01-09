#================================================================
#
# Configure libthread_db
#
#   ----------------------------------------
#
#  thread_db is provided by glibc, so there is no hint variable
#
#================================================================

include_guard(GLOBAL)

# thread_db is only available on Unixes; provide a dummy target on other platforms
if(NOT UNIX)
  if(NOT TARGET Dyninst::Thread_DB)
    add_library(Dyninst::Thread_DB INTERFACE)
  endif()
  return()
endif()

find_package(Thread_DB)

# It's not required, so just make a dummy target if not found
if(NOT Thread_DB_FOUND)
  if(NOT TARGET Dyninst::Thread_DB)
    add_library(Dyninst::Thread_DB INTERFACE IMPORTED)
  endif()
  return()
endif()

if(NOT TARGET Dyninst::Thread_DB)
  add_library(Dyninst::Thread_DB INTERFACE IMPORTED)
  target_include_directories(Dyninst::Thread_DB SYSTEM
                             INTERFACE ${Thread_DB_INCLUDE_DIRS})
  target_link_libraries(Dyninst::Thread_DB INTERFACE Thread_DB::Thread_DB)
  target_compile_definitions(Dyninst::Thread_DB INTERFACE cap_thread_db)
endif()
