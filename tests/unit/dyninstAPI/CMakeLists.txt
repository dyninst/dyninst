include_guard(GLOBAL)

# dyninstAPI is not built with LIGHTWEIGHT_SYMTAB
if(LIGHTWEIGHT_SYMTAB)
  return()
endif()

add_subdirectory(emitter)
