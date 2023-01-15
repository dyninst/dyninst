include_guard(GLOBAL)

include(DyninstLibrary)

if(NOT DYNINST_OS_UNIX)
  if(NOT TARGET dynDwarf)
    add_library(dynDwarf INTERFACE)
  endif()
  return()
endif()

set(_public_headers h/dwarfExprParser.h h/dwarfFrameParser.h h/dwarfHandle.h
                    h/dwarfResult.h)

set(_sources src/dwarfResult.C src/dwarfExprParser.C src/dwarfFrameParser.C
             src/dwarfHandle.C)

# cmake-format: off
dyninst_library(
  dynDwarf
  PUBLIC_HEADER_FILES ${_public_headers}
  SOURCE_FILES ${_sources}
  DEFINES DYNDWARF_LIB
  DYNINST_DEPS dynElf common
  PUBLIC_DEPS Dyninst::Boost_headers
)
# cmake-format: on

