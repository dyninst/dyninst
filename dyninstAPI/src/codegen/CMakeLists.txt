include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers codegen.h RegControl.h)

set(_sources codegen.C)

if(DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64)
  list(
    APPEND _headers
    codegen-x86.h
    emitters/x86/Emitterx86.h
    emitters/x86/generators.h
    emitters/x86/IA32/EmitterIA32Dyn.h
    emitters/x86/IA32/EmitterIA32.h
    emitters/x86/IA32/EmitterIA32Stat.h
  )

  list(
    APPEND _sources
    codegen-x86.C
    emitters/x86/Emitterx86.C
    emitters/x86/generators.C
    emitters/x86/IA32/EmitterIA32.C
    emitters/x86/IA32/EmitterIA32Dyn.C
    emitters/x86/IA32/EmitterIA32Stat.C
  )

elseif(DYNINST_CODEGEN_ARCH_AARCH64)
  list(
    APPEND _headers
    codegen-aarch64.h
    emitters/aarch64/EmitterAarch64.h
    emitters/aarch64/EmitterAarch64Dyn.h
    emitters/aarch64/EmitterAarch64Stat.h
    emitters/aarch64/EmitterAarch64RestoreRegs.h
    emitters/aarch64/EmitterAarch64SaveRegs.h
    emitters/aarch64/generators.h
  )

  list(
    APPEND _sources
    codegen-aarch64.C
    emitters/aarch64/EmitterAarch64.C
    emitters/aarch64/EmitterAarch64Stat.C
    emitters/aarch64/EmitterAarch64RestoreRegs.C
    emitters/aarch64/EmitterAarch64SaveRegs.C
    emitters/aarch64/generators.C
  )

elseif(DYNINST_CODEGEN_ARCH_PPC64LE)
  list(
    APPEND _headers
    codegen-power.h
    emitters/PowerPC/EmitterPowerPC.h
    emitters/PowerPC/generators.h
    emitters/PowerPC/ppc32/EmitterPowerPC32Dyn.h
    emitters/PowerPC/ppc32/EmitterPowerPC32Stat.h
    emitters/PowerPC/ppc64/EmitterPowerPC64Dyn.h
    emitters/PowerPC/ppc64/EmitterPowerPC64Stat.h
  )

  list(
    APPEND _sources
    codegen-power.C
    emitters/PowerPC/EmitterPowerPC.C
    emitters/PowerPC/generators.C
    emitters/PowerPC/ppc32/EmitterPowerPC32Stat.C
    emitters/PowerPC/ppc64/EmitterPowerPC64Dyn.C
    emitters/PowerPC/ppc64/EmitterPowerPC64Stat.C
  )

elseif(
  DYNINST_CODEGEN_ARCH_AMDGPU_GFX908
  OR DYNINST_CODEGEN_ARCH_AMDGPU_GFX90A
  OR DYNINST_CODEGEN_ARCH_AMDGPU_GFX940
  OR DYNINST_CODEGEN_ARCH_AMDGPU_GFX950
)
  list(
    APPEND _headers
    codegen-amdgpu.h
    emitters/amdgpu/emit-amdgpu.h
    emitters/amdgpu/amdgpu-encode.hpp
    emitters/amdgpu/gfx908/emit-amdgpu-gfx908.h
    emitters/amdgpu/gfx908/amdgpu-gfx908-details.h
    emitters/amdgpu/gfx908/amdgpu-gfx908-encoding.h
    emitters/amdgpu/gfx90a/amdgpu-gfx90a-encoding.h
    emitters/amdgpu/gfx940/emit-amdgpu-gfx940.h
    emitters/amdgpu/gfx940/amdgpu-gfx940-details.h
    emitters/amdgpu/gfx940/amdgpu-gfx940-encoding.h
    emitters/amdgpu/gfx950/amdgpu-gfx950-encoding.h
  )

  list(
    APPEND _sources
    codegen-amdgpu.C
    emitters/amdgpu/emit-amdgpu.C
    emitters/amdgpu/gfx908/emit-amdgpu-gfx908.C
    emitters/amdgpu/gfx908/amdgpu-gfx908-details.C
    # Additional codegen gens compiled alongside for runtime emitter selection (getEmitter() by object arch).
    emitters/amdgpu/gfx940/emit-amdgpu-gfx940.C
    emitters/amdgpu/gfx940/amdgpu-gfx940-details.C
  )
endif()

dyninst_library(
  dyninstAPI_codegen
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_DEPS common patchAPI pcontrol
)

set(
  _inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../ASTs
  ${CMAKE_CURRENT_SOURCE_DIR}/../registerSpace
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)

target_include_directories(dyninstAPI_codegen PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
