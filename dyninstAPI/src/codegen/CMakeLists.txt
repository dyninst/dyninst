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

elseif(DYNINST_CODEGEN_ARCH_AARCH64)
  list(
    APPEND
    _headers
    emitters/aarch64/EmitterAarch64.h
    emitters/aarch64/EmitterAarch64Dyn.h
    emitters/aarch64/EmitterAarch64Stat.h
    emitters/aarch64/EmitterAarch64RestoreRegs.h
    emitters/aarch64/EmitterAarch64SaveRegs.h
    emitters/aarch64/generators.h)

  list(
    APPEND
    _sources
    emitters/aarch64/EmitterAarch64.C
    emitters/aarch64/EmitterAarch64Stat.C
    emitters/aarch64/EmitterAarch64RestoreRegs.C
    emitters/aarch64/EmitterAarch64SaveRegs.C
    emitters/aarch64/generators.C)

endif()

# cmake-format: off
dyninst_library(
  dyninstAPI_codegen
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common patchAPI pcontrol
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../ASTs
  ${CMAKE_CURRENT_SOURCE_DIR}/../registerSpace
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)
# cmake-format: on

target_include_directories(dyninstAPI_codegen PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
