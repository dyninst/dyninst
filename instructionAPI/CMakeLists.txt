include_guard(GLOBAL)

include(DyninstLibrary)

set(_sources
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
    src/AMDGPU/vega/InstructionDecoder-amdgpu-vega.C
    src/AMDGPU/cdna2/InstructionDecoder-amdgpu-cdna2.C
    src/InstructionDecoderImpl.C)

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
	h/RegisterIDs.h
	h/Result.h
	h/Ternary.h
	h/Visitor.h)

set(_private_headers
	src/amdgpu_branchinsn_table.h
	src/AMDGPU/cdna2/amdgpu_cdna2_decoder_impl.h
	src/AMDGPU/cdna2/amdgpu_cdna2_insn_entry.h
	src/AMDGPU/cdna2/decodeOperands.h
	src/AMDGPU/cdna2/InstructionDecoder-amdgpu-cdna2.h
	src/AMDGPU/vega/amdgpu_vega_decoder_impl.h
	src/AMDGPU/vega/amdgpu_vega_insn_entry.h
	src/AMDGPU/vega/InstructionDecoder-amdgpu-vega.h
	src/InstructionDecoder-aarch64.h
	src/InstructionDecoderImpl.h
	src/InstructionDecoder-power.h
	src/InstructionDecoder-x86.h)

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
        APPEND
        instructionDecoderPowerExtraFlags
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
        APPEND
        finalizeOperandsExtraFlags
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
    set_source_files_properties(src/AMDGPU/cdna2/InstructionDecoder-amdgpu-cdna2.C
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
	${CMAKE_CURRENT_SOURCE_DIR}/h
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/AMDGPU/cdna2
	${CMAKE_CURRENT_SOURCE_DIR}/src/AMDGPU/vega)

foreach(t ${instructionAPI_TARGETS})
  target_include_directories(${t} PUBLIC "$<BUILD_INTERFACE:${_inc_dirs}>")
endforeach()
unset(_inc_dirs)
