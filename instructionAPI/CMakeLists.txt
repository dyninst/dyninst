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
     src/Expression.C 
     src/BinaryFunction.C 
     src/InstructionCategories.C
     src/ArchSpecificFormatters.C
     src/Immediate.C 
     src/InstructionDecoder.C 
     src/InstructionDecoder-x86.C
     src/InstructionDecoder-power.C 
     src/InstructionDecoder-aarch64.C 
     src/InstructionDecoderImpl.C
  )
SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

# Shave 3 minute off of the time it takes to compile this file by falling back to not tracking
# variables by default, rather then waiting until the variable tracking limit
# is reached.
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
set_source_files_properties(src/InstructionDecoder-aarch64.C PROPERTIES COMPILE_FLAGS
    -fno-var-tracking-assignments)
endif()

ADD_DEFINITIONS(-DINSTRUCTION_LIB)

dyninst_library(instructionAPI common)

target_link_private_libraries(instructionAPI ${Boost_LIBRARIES})

if (USE_COTIRE)
    cotire(instructionAPI)
endif()
