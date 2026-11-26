include_guard(GLOBAL)

# Exported functions calling each other through PLT stubs whose jump slots bind
# to definitions in the same library. A call through a stub whose definition
# never returns must have no fallthrough, and a tail call through it must not
# make the caller return.
add_library(plt_noret SHARED fatal_inner.c fatal.c ok.c callers.c)
target_compile_options(plt_noret PRIVATE -O2 -fPIC)
target_link_options(plt_noret PRIVATE -nostdlib)
dyninst_add_test(
  parseAPI_PltNoReturn
  TYPE regression
  SOURCE_FILES pltNoReturn.cpp
  DEPS parseAPI
  ARGS $<TARGET_FILE:plt_noret>
)
