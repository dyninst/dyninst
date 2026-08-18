include_guard(GLOBAL)

if(NOT DYNINST_ENABLE_CAPSTONE)
  add_library(instructionAPI_riscv INTERFACE)
  return()
endif()

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers categories.h decoder.h mem_xlat.h opcode_xlat.h register_xlat.h)

set(_sources decoder.C mem_xlat.C opcode_xlat.C register_xlat.C)

# cmake-format: off
dyninst_library(
  instructionAPI_riscv
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common
  PUBLIC_DEPS Dyninst::Capstone
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${PROJECT_SOURCE_DIR}/instructionAPI/h)
# cmake-format: on

target_include_directories(instructionAPI_riscv PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
