include_guard(GLOBAL)

include(DyninstLibrary)

set(_public_headers
    h/AddrLookup.h
    h/Aggregate.h
    h/Archive.h
    h/Collections.h
    h/Function.h
    h/LineInformation.h
    h/Module.h
    h/RangeLookup.h
    h/Region.h
    h/StringTable.h
    h/Symbol.h
    h/Symtab.h
    h/SymtabReader.h
    h/symutil.h
    h/Type.h
    h/Variable.h)

set(_private_headers
    src/annotations.h
    src/debug.h
    src/dwarfWalker.h
    src/emitElf.h
    src/emitElfStatic.h
    src/emitWin.h
    src/LinkMap.h
    src/Object-elf.h
    src/Object.h
    src/Object-nt.h
    src/Type-mem.h)

set(_sources
    src/AddrLookup.C
    src/Aggregate.C
    src/annotations.C
    src/Collections.C
    src/debug.C
    src/Function.C
    src/LineInformation.C
    src/Module.C
    src/Object.C
    src/Region.C
    src/Symbol.C
    src/Symtab-edit.C
    src/Symtab-lookup.C
    src/Symtab.C
    src/SymtabReader.C
    src/Type.C
    src/Variable.C)

if(DYNINST_OS_UNIX)
  list(
    APPEND
    _sources
    src/Object-elf.C
    src/Archive.C
    src/Archive-elf.C
    src/parseDwarf.C
    src/LinkMap.C
    src/emitElf.C
    src/emitElfStatic.C
    src/dwarfWalker.C)

  if(DYNINST_ARCH_x86_64 OR DYNINST_ARCH_i386)
    list(APPEND _sources src/emitElfStatic-x86.C src/relocationEntry-elf-x86.C)
  elseif(DYNINST_ARCH_ppc64le)
    list(APPEND _sources src/emitElfStatic-ppc64.C src/relocationEntry-elf-ppc64.C)
  elseif(DYNINST_ARCH_aarch64)
    list(APPEND _sources src/emitElfStatic-aarch64.C src/relocationEntry-elf-aarch64.C)
  else()
    list(APPEND _sources src/emitElfStatic-stub.C src/relocationEntry-stub.C)
  endif()
elseif(DYNINST_OS_Windows)
  list(APPEND _sources src/Object-nt.C src/emitWin.C src/relocationEntry-stub.C)
endif()

# cmake-format: off
dyninst_library(
  symtabAPI
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DEFINES SYMTAB_LIB
  PUBLIC_DEPS common dynElf dynDwarf Dyninst::ElfUtils Dyninst::Boost 
  PRIVATE_DEPS Dyninst::TBB OpenMP::OpenMP_CXX
)
# cmake-format: on

foreach(t ${symtabAPI_TARGETS})
  target_include_directories(
    ${t}
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/h>")

  if(DYNINST_OS_Windows)
    target_link_libraries(${t} PRIVATE dbghelp)
  endif()
endforeach()
