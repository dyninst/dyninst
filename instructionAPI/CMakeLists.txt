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
     src/Immediate.C 
     src/InstructionDecoder.C 
     src/InstructionDecoder-power.C 
     src/InstructionDecoderImpl.C
  )

add_library (instructionAPI ${SRC_LIST})

FILE (GLOB includes "${CMAKE_CURRENT_SOURCE_DIR}/h")
INSTALL (FILES ${includes} DESTINATION include)
