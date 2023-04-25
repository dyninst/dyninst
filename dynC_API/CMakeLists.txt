include_guard(GLOBAL)

include(DyninstLibrary)

# dynC requires symtabAPI
if(LIGHTWEIGHT_SYMTAB)
  message(STATUS "LIGHTWEIGHT_SYMTAB enabled; dynC_API not built.")
  return()
endif()

set(_public_headers h/dynC.h h/snippetGen.h)

set(_private_headers src/dynC.tab.h)

set(_sources src/dynC.C src/dynC.tab.C src/lex.dynC.C src/snippetGen.C)

# cmake-format: off
dyninst_library(
  dynC_API
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DEFINES DYNC_EXPORTS
  DYNINST_DEPS common dyninstAPI symtabAPI
)
# cmake-format: on

if(DYNINST_OS_Windows)
  foreach(t ${dynC_API_TARGETS})
    target_compile_definitions(${t} PRIVATE "YY_NO_UNISTD_H")
  endforeach()
endif()
