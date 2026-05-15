include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers baseTramp.h baseTramp-aarch64.h baseTramp-amdgpu.h baseTramp-ppc.h
             baseTramp-riscv64.h baseTramp-x86.h)

set(_sources baseTramp.C)

if(DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64)
  list(APPEND _sources baseTramp-x86.C)
elseif(DYNINST_CODEGEN_ARCH_PPC64LE)
  list(APPEND _sources baseTramp-ppc.C)
elseif(DYNINST_CODEGEN_ARCH_AARCH64)
  list(APPEND _sources baseTramp-aarch64.C)
endif()

add_library(dyninstAPI_trampolines INTERFACE)
target_sources(dyninstAPI_trampolines INTERFACE ${_headers} ${_sources})
target_include_directories(dyninstAPI_trampolines INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

unset(_headers)
unset(_sources)
