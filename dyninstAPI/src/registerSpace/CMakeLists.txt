include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers RegisterConversion.h registerSpace.h RealRegister.h registerSlot.h)

set(_sources registerSpace.C registerSlot.C)

if(DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64)
  list(APPEND _sources RegisterConversion-x86.C registerSpace-x86.C)
elseif(DYNINST_CODEGEN_ARCH_PPC64LE)
  list(APPEND _sources RegisterConversion-ppc.C registerSpace-ppc.C)
elseif(DYNINST_CODEGEN_ARCH_AARCH64)
  list(APPEND _sources RegisterConversion-aarch64.C registerSpace-aarch64.C)
elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
  list(APPEND _sources RegisterConversion-amdgpu.C registerSpace-amdgpu.C)
endif()

# cmake-format: off
dyninst_library(
  dyninstAPI_registerSpace
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common parseAPI patchAPI pcontrol
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../ASTs
  ${CMAKE_CURRENT_SOURCE_DIR}/../BPatch
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)
# cmake-format: on

target_include_directories(dyninstAPI_registerSpace
                           PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
