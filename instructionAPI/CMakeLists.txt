include_guard(GLOBAL)

include(DyninstLibrary)

set(_sources
    src/debug.C
    src/Instruction.C
    src/InstructionAST.C
    src/Operation.C
    src/Operand.C
    src/Register.C
    src/Ternary.C
    src/Expression.C
    src/BinaryFunction.C
    src/InstructionCategories.C
    src/ArchSpecificFormatters.C
    src/Immediate.C
    src/InstructionDecoder.C
    src/InstructionDecoder-x86.C
    src/InstructionDecoder-power.C
    src/InstructionDecoder-aarch64.C
    src/AMDGPU/gfx908/decodeOperands.C
    src/AMDGPU/gfx908/appendOperands.C
    src/AMDGPU/gfx908/finalizeOperands.C
    src/AMDGPU/gfx908/amdgpu_gfx908_decoder_impl.C
    src/AMDGPU/gfx908/InstructionDecoder-amdgpu-gfx908.C
    src/AMDGPU/gfx90a/decodeOperands.C
    src/AMDGPU/gfx90a/appendOperands.C
    src/AMDGPU/gfx90a/finalizeOperands.C
    src/AMDGPU/gfx90a/amdgpu_gfx90a_decoder_impl.C
    src/AMDGPU/gfx90a/InstructionDecoder-amdgpu-gfx90a.C
    src/AMDGPU/gfx940/decodeOperands.C
    src/AMDGPU/gfx940/appendOperands.C
    src/AMDGPU/gfx940/finalizeOperands.C
    src/AMDGPU/gfx940/amdgpu_gfx940_decoder_impl.C
    src/AMDGPU/gfx940/InstructionDecoder-amdgpu-gfx940.C
    src/InstructionDecoderImpl.C
    src/syscalls.C
    src/interrupts.C)

set(_public_headers
    h/ArchSpecificFormatters.h
    h/BinaryFunction.h
    h/Dereference.h
    h/Expression.h
    h/Immediate.h
    h/InstructionAST.h
    h/InstructionCategories.h
    h/InstructionDecoder.h
    h/Instruction.h
    h/Operand.h
    h/Operation_impl.h
    h/Register.h
    h/Result.h
    h/Ternary.h
    h/Visitor.h)

set(_private_headers
    src/debug.h
    src/amdgpu_branchinsn_table.h
    src/AMDGPU/gfx940/InstructionDecoder-amdgpu-gfx940.h
    src/AMDGPU/gfx90a/InstructionDecoder-amdgpu-gfx90a.h
    src/AMDGPU/gfx908/InstructionDecoder-amdgpu-gfx908.h
    src/InstructionDecoder-aarch64.h
    src/InstructionDecoderImpl.h
    src/InstructionDecoder-power.h
    src/InstructionDecoder-x86.h
    h/syscalls.h
    h/interrupts.h)

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
    src/InstructionDecoder-power.C PROPERTIES COMPILE_FLAGS
                                              "${instructionDecoderPowerExtraFlags}")
endif()

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
  set_source_files_properties(src/AMDGPU/gfx940/InstructionDecoder-amdgpu-gfx940.C
                              PROPERTIES COMPILE_FLAGS "${finalizeOperandsExtraFlags}")
  set_source_files_properties(src/AMDGPU/gfx90a/InstructionDecoder-amdgpu-gfx90a.C
                              PROPERTIES COMPILE_FLAGS "${finalizeOperandsExtraFlags}")
  set_source_files_properties(src/AMDGPU/gfx908/InstructionDecoder-amdgpu-gfx908.C
                              PROPERTIES COMPILE_FLAGS "${finalizeOperandsExtraFlags}")
endif()

# cmake-format: off
dyninst_library(
  instructionAPI
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DEFINES INSTRUCTION_LIB
  DYNINST_DEPS common
  PUBLIC_DEPS Dyninst::Boost_headers
)
# cmake-format: on

set(_inc_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AMDGPU/gfx940
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AMDGPU/gfx90a
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AMDGPU/gfx908)

foreach(t ${instructionAPI_TARGETS})
  target_include_directories(${t} PUBLIC "$<BUILD_INTERFACE:${_inc_dirs}>")
endforeach()
unset(_inc_dirs)
