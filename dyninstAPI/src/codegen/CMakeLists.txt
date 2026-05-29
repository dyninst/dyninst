include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers RegControl.h)

if(DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64)
  list(
    APPEND
    _headers
    emitters/x86/Emitterx86.h
    emitters/x86/generators.h
    emitters/x86/IA32/EmitterIA32Dyn.h
    emitters/x86/IA32/EmitterIA32.h
    emitters/x86/IA32/EmitterIA32Stat.h)

  list(
    APPEND
    _sources
    emitters/x86/Emitterx86.C
    emitters/x86/generators.C
    emitters/x86/IA32/EmitterIA32.C
    emitters/x86/IA32/EmitterIA32Dyn.C
    emitters/x86/IA32/EmitterIA32Stat.C)
endif()

add_library(dyninstAPI_codegen INTERFACE)
target_sources(dyninstAPI_codegen INTERFACE ${_headers} ${_sources})
target_include_directories(dyninstAPI_codegen INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

unset(_headers)
unset(_sources)
