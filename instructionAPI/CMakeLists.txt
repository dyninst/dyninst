# CMake configuration for instructionAPI directory

include_directories (
    ${PROJECT_SOURCE_DIR}/instructionAPI/src
  )


set (SRC_LIST
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
     src/InstructionDecoder-amdgpu-vega.C
     src/InstructionDecoderImpl.C
  )
SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

set(instructionDecoderPowerExtraFlags "")
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (${CMAKE_CXX_COMPILER_VERSION} MATCHES "^([1-9]|1[01])(\.|$)")
        # Disable var-tracking-assignments for InstructionDecoder-power.C for
        # all known versions of gcc..  The default max size is too small so it
        # fails, and adjusting it using
        #
        #   PROPERTIES COMPILE_FLAGS "--param=max-vartrack-size=900000000"
        #
        # succeeds, but the object file produced is >1GB, so disable it.
        string(APPEND instructionDecoderPowerExtraFlags "-fno-var-tracking-assignments")
    endif()
endif()

# adjust warning threshold if set in cmake/warnings.cmake
if (debugMaxFrameSizeOverridePowerOpcodeTable)
        string(APPEND instructionDecoderPowerExtraFlags
                " $<$<CONFIG:DEBUG>:-Wframe-larger-than=${debugMaxFrameSizeOverridePowerOpcodeTable}>")
endif()
if (nonDebugMaxFrameSizeOverridePowerOpcodeTable)
        string(APPEND instructionDecoderPowerExtraFlags
                " $<$<NOT:$<CONFIG:DEBUG>>:-Wframe-larger-than=${nonDebugMaxFrameSizeOverridePowerOpcodeTable}>")
endif()

if (NOT instructionDecoderPowerExtraFlags STREQUAL "")
    SET_SOURCE_FILES_PROPERTIES(src/InstructionDecoder-power.C
            PROPERTIES COMPILE_FLAGS "${instructionDecoderPowerExtraFlags}")
endif()

ADD_DEFINITIONS(-DINSTRUCTION_LIB)

dyninst_library(instructionAPI common)

target_link_private_libraries(instructionAPI ${Boost_LIBRARIES} ${TBB_LIBRARIES} tbbmalloc)
