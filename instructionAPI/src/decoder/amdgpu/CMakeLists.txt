include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(
  _headers
  amdgpu_branchinsn_table.h
  gfx908/InstructionDecoder-amdgpu-gfx908.h
  gfx90a/InstructionDecoder-amdgpu-gfx90a.h
  gfx942/InstructionDecoder-amdgpu-gfx942.h
  gfx950/InstructionDecoder-amdgpu-gfx950.h
)

set(
  _sources
  gfx908/decodeOperands.C
  gfx908/appendOperands.C
  gfx908/finalizeOperands.C
  gfx908/amdgpu_gfx908_decoder_impl.C
  gfx908/InstructionDecoder-amdgpu-gfx908.C
  gfx90a/decodeOperands.C
  gfx90a/appendOperands.C
  gfx90a/finalizeOperands.C
  gfx90a/amdgpu_gfx90a_decoder_impl.C
  gfx90a/InstructionDecoder-amdgpu-gfx90a.C
  gfx942/decodeOperands.C
  gfx942/appendOperands.C
  gfx942/finalizeOperands.C
  gfx942/amdgpu_gfx942_decoder_impl.C
  gfx942/InstructionDecoder-amdgpu-gfx942.C
  gfx950/decodeOperands.C
  gfx950/appendOperands.C
  gfx950/finalizeOperands.C
  gfx950/amdgpu_gfx950_decoder_impl.C
  gfx950/InstructionDecoder-amdgpu-gfx950.C
)

dyninst_library(
  instructionAPI_amdgpu
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_DEPS common
)

set(
  _inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${PROJECT_SOURCE_DIR}/instructionAPI/h
)

target_include_directories(instructionAPI_amdgpu PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
