include_guard(GLOBAL)

if(DYNINST_OS_UNIX)
  set(_required REQUIRED)
endif()

find_package(Threads ${_required})

if(NOT Threads_FOUND)
  # make a dummy
  add_library(Threads::Threads INTERFACE)
endif()
