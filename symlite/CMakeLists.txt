include_guard(GLOBAL)

if(NOT LIGHTWEIGHT_SYMTAB)
  return()
endif()

include(DyninstLibrary)

if(NOT DYNINST_OS_UNIX)
  if(NOT TARGET symLite)
    add_library(symLite INTERFACE)
  endif()
  return()
endif()

set(_public_headers h/SymLite-elf.h)

set(_sources src/SymLite-elf.C)

# cmake-format: off
dyninst_library(
  symLite
  PUBLIC_HEADER_FILES ${_public_headers}
  SOURCE_FILES ${_sources}
  DEFINES SYMLITE_LIB
  DYNINST_DEPS common dynElf
)
# cmake-format: on
