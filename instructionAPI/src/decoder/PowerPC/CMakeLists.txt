include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers InstructionDecoder-power.h power_opcode_tables.h)

set(_sources InstructionDecoder-power.C power_opcode_tables.C)

set(instructionDecoderPowerExtraFlags "")
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  if(${CMAKE_CXX_COMPILER_VERSION} MATCHES "^([1-9]|1[01])(\.|$)")
    # Disable var-tracking-assignments for InstructionDecoder-power.C for all known
    # versions of gcc..  The default max size is too small so it fails, and adjusting
    # it using
    #
    # PROPERTIES COMPILE_FLAGS "--param=max-vartrack-size=900000000"
    #
    # succeeds, but the object file produced is >1GB, so disable it.
    string(APPEND instructionDecoderPowerExtraFlags "-fno-var-tracking-assignments")
  endif()
endif()

# adjust warning threshold if set in cmake/warnings.cmake
if(debugMaxFrameSizeOverridePowerOpcodeTable)
  string(
    APPEND instructionDecoderPowerExtraFlags
    " $<$<CONFIG:DEBUG>:-Wframe-larger-than=${debugMaxFrameSizeOverridePowerOpcodeTable}>"
    )
endif()
if(nonDebugMaxFrameSizeOverridePowerOpcodeTable)
  string(
    APPEND
    instructionDecoderPowerExtraFlags
    " $<$<NOT:$<CONFIG:DEBUG>>:-Wframe-larger-than=${nonDebugMaxFrameSizeOverridePowerOpcodeTable}>"
    )
endif()

if(NOT instructionDecoderPowerExtraFlags STREQUAL "")
  set_source_files_properties(
    power_opcode_tables.C PROPERTIES COMPILE_FLAGS "${instructionDecoderPowerExtraFlags}")
endif()

# cmake-format: off
dyninst_library(
  instructionAPI_powerpc
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${PROJECT_SOURCE_DIR}/instructionAPI/h)
# cmake-format: on

target_include_directories(instructionAPI_powerpc PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
