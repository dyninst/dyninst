include_guard(GLOBAL)

include(DyninstLibrary)

set(_public_headers
    h/basetypes.h
    h/frame.h
    h/framestepper.h
    h/local_var.h
    h/procstate.h
    h/steppergroup.h
    h/swk_errors.h
    h/symlookup.h
    h/walker.h)

set(_private_headers
    src/addrRange.h
    src/aarch64-swk.h
    src/analysis_stepper.h
    src/dbgstepper-impl.h
    src/framestepper_pimple.h
    src/freebsd-swk.h
    src/get_trap_instruction.h
    src/libstate.h
    src/linuxbsd-swk.h
    src/linux-swk.h
    src/sw.h
    src/symtab-swk.h
    src/x86-swk.h)

set(_sources
    src/frame.C
    src/framestepper.C
    src/swk_errors.C
    src/symlookup.C
    src/walker.C
    src/procstate.C
    src/steppergroup.C
    src/libstate.C
    src/sw_pcontrol.C
    src/symtab-swk.C)

if(DYNINST_ARCH_i386 OR DYNINST_ARCH_x86_64)
  list(APPEND _sources src/x86-swk.C src/x86-wanderer.C)
endif()

if(DYNINST_OS_FreeBSD)
  list(APPEND _sources src/freebsd-swk.C)
elseif(DYNINST_OS_Windows)
  list(APPEND _sources src/win-x86-swk.C)
endif()

if(DYNINST_OS_UNIX)
  list(APPEND _sources src/linuxbsd-swk.C src/linux-swk.C)
  if(DYNINST_ARCH_ppc64le)
    list(APPEND _sources src/linux-ppc-swk.C src/ppc-swk.C)
  elseif(DYNINST_ARCH_aarch64)
    list(APPEND _sources src/linux-aarch64-swk.C src/aarch64-swk.C src/dbginfo-stepper.C)
  elseif(DYNINST_ARCH_i386 OR DYNINST_ARCH_x86_64)
    list(APPEND _sources src/linux-x86-swk.C src/linuxbsd-x86-swk.C src/dbginfo-stepper.C)
    if(DYNINST_OS_FreeBSD)
      list(APPEND _sources src/freebsd-x86-swk.C)
    endif()
  endif()
endif()

# cmake-format: off
dyninst_library(
  stackwalk
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DEFINES STACKWALKER_EXPORTS
  DYNINST_DEPS common dynDwarf dynElf instructionAPI pcontrol ${SYMREADER}
  PUBLIC_DEPS Dyninst::Boost_headers
  PRIVATE_DEPS Dyninst::ElfUtils
)
# cmake-format: on

foreach(t ${stackwalk_TARGETS})
  # Force frame pointers in stackwalker, no matter the optimization level
  # See commit 2608333f7
  target_compile_options(${t} PRIVATE ${DYNINST_FORCE_FRAME_POINTER})

  if(SW_ANALYSIS_STEPPER)
    target_compile_definitions(${t} PRIVATE USE_PARSE_API)
    if(${t} MATCHES "static")
      target_link_libraries(${t} PUBLIC parseAPI_static)
    else()
      target_link_libraries(${t} PUBLIC parseAPI)
    endif()
    target_sources(${t} PRIVATE src/analysis_stepper.C src/callchecker-IAPI.C)
  else()
    target_sources(${t} PRIVATE src/callchecker.C)
  endif()
endforeach()
