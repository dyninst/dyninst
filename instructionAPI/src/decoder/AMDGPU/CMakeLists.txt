include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers
    amdgpu_branchinsn_table.h gfx908/InstructionDecoder-amdgpu-gfx908.h
    gfx90a/InstructionDecoder-amdgpu-gfx90a.h gfx940/InstructionDecoder-amdgpu-gfx940.h
    gfx950/InstructionDecoder-amdgpu-gfx950.h)

set(_sources
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
    gfx940/decodeOperands.C
    gfx940/appendOperands.C
    gfx940/finalizeOperands.C
    gfx940/amdgpu_gfx940_decoder_impl.C
    gfx940/InstructionDecoder-amdgpu-gfx940.C
    gfx950/decodeOperands.C
    gfx950/appendOperands.C
    gfx950/finalizeOperands.C
    gfx950/amdgpu_gfx950_decoder_impl.C
    gfx950/InstructionDecoder-amdgpu-gfx950.C)

set(finalizeOperandsExtraFlags "")
if(debugMaxFrameSizeOverrideFinalizeOperands)
  string(
    APPEND finalizeOperandsExtraFlags
    " $<$<CONFIG:DEBUG>:-Wframe-larger-than=${debugMaxFrameSizeOverrideFinalizeOperands}>"
    )
endif()
if(nonDebugMaxFrameSizeOverrideFinalizeOperands)
  string(
    APPEND
    finalizeOperandsExtraFlags
    " $<$<NOT:$<CONFIG:DEBUG>>:-Wframe-larger-than=${nonDebugMaxFrameSizeOverrideFinalizeOperands}>"
    )
endif()

if(NOT finalizeOperandsExtraFlags STREQUAL "")
  set_source_files_properties(gfx908/InstructionDecoder-amdgpu-gfx908.C
                              PROPERTIES COMPILE_FLAGS "${finalizeOperandsExtraFlags}")
  set_source_files_properties(gfx90a/InstructionDecoder-amdgpu-gfx90a.C
                              PROPERTIES COMPILE_FLAGS "${finalizeOperandsExtraFlags}")
  set_source_files_properties(gfx940/InstructionDecoder-amdgpu-gfx940.C
                              PROPERTIES COMPILE_FLAGS "${finalizeOperandsExtraFlags}")
  set_source_files_properties(gfx950/InstructionDecoder-amdgpu-gfx950.C
                              PROPERTIES COMPILE_FLAGS "${finalizeOperandsExtraFlags}")
endif()

# cmake-format: off
dyninst_library(
  instructionAPI_amdgpu
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

target_include_directories(instructionAPI_amdgpu PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
