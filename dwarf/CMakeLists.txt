include_guard(GLOBAL)

include(DyninstLibrary)

if(NOT DYNINST_OS_UNIX)
  if(NOT TARGET dynDwarf)
    add_library(dynDwarf INTERFACE)
  endif()
  return()
endif()

set(_public_headers h/dwarfExprParser.h h/dwarfFrameParser.h h/dwarfHandle.h
                    h/dwarfResult.h h/dwarf_unit_info.h)

set(_sources src/dwarfResult.C src/dwarfExprParser.C src/dwarfFrameParser.C
             src/dwarfHandle.C src/dwarf_subrange.cpp src/registers/convert.C)

set(_private_headers
    src/registers/convert.h
    src/registers/x86.h
    src/registers/x86_64.h
    src/registers/ppc32.h
    src/registers/ppc64.h
    src/registers/aarch64.h
    src/registers/riscv64.h)

# cmake-format: off
dyninst_library(
  dynDwarf
  PRIVATE_HEADER_FILES ${_private_headers}
  PUBLIC_HEADER_FILES ${_public_headers}
  SOURCE_FILES ${_sources}
  DEFINES DYNDWARF_LIB
  DYNINST_DEPS dynElf common
  PRIVATE_DEPS Dyninst::Boost_headers Dyninst::ElfUtils
  INTERNAL_LIBRARY
)
# cmake-format: on
