include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers InstructionDecoder-power.h power_opcode_tables.h)

set(_sources InstructionDecoder-power.C power_opcode_tables.C)

dyninst_library(
  instructionAPI_powerpc
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

target_include_directories(instructionAPI_powerpc PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
