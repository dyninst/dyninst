include_guard(GLOBAL)

# StackwalkerAPI has no SymtabAPI support with LIGHTWEIGHT_SYMTAB
if(LIGHTWEIGHT_SYMTAB)
  return()
endif()

add_subdirectory(vdso)
