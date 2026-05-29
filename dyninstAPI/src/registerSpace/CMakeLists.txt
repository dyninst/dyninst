include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers registerSpace.h RealRegister.h registerSlot.h)

set(_sources registerSpace.C registerSlot.C)

if(DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64)
  list(APPEND _sources registerSpace-x86.C)
elseif(DYNINST_CODEGEN_ARCH_PPC64LE)
  list(APPEND _sources registerSpace-ppc.C)
elseif(DYNINST_CODEGEN_ARCH_AARCH64)
  list(APPEND _sources registerSpace-aarch64.C)
elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
  list(APPEND _sources registerSpace-amdgpu.C)
endif()

add_library(dyninstAPI_registerSpace INTERFACE)
target_sources(dyninstAPI_registerSpace INTERFACE ${_headers} ${_sources})
target_include_directories(dyninstAPI_registerSpace INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

unset(_headers)
unset(_sources)
