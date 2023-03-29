include_guard(GLOBAL)

include(DyninstLibrary)

if(NOT DYNINST_OS_UNIX)
  if(NOT TARGET dynElf)
    add_library(dynElf INTERFACE)
  endif()
  return()
endif()

set(_public_headers h/Elf_X.h)
set(_sources src/Elf_X.C)

# cmake-format: off
dyninst_library(
  dynElf
  PUBLIC_HEADER_FILES ${_public_headers}
  SOURCE_FILES ${_sources}
  DEFINES DYNELF_LIB
  DYNINST_DEPS common
  PRIVATE_DEPS Dyninst::Boost_headers
  PUBLIC_DEPS Dyninst::ElfUtils
)
# cmake-format: on
