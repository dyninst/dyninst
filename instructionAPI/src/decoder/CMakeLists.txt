include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers type_conversion.h InstructionDecoderImpl.h)

set(_sources InstructionCategories.C InstructionDecoder.C InstructionDecoderImpl.C)

# cmake-format: off
dyninst_library(
  instructionAPI_decoder
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS
    common
    instructionAPI_aarch64
    instructionAPI_amdgpu
    instructionAPI_powerpc
    instructionAPI_riscv
    instructionAPI_x86
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/instructionAPI/h)
# cmake-format: on

target_include_directories(instructionAPI_decoder PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
