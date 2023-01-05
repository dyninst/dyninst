include_guard(GLOBAL)

include(DyninstLibrary)

# dynC requires symtabAPI
if(LIGHTWEIGHT_SYMTAB)
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
  PUBLIC_DEPS common dyninstAPI symtabAPI
)
# cmake-format: on

foreach(t ${dynC_API_TARGETS})
  target_include_directories(
    ${t}
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/h>")

  if(DYNINST_OS_Windows)
    target_compile_definitions(${t} PRIVATE "YY_NO_UNISTD_H")
  endif()
endforeach()
