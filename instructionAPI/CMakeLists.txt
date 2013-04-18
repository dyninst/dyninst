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
     src/InstructionDecoder-x86.C
     src/InstructionDecoder-power.C 
     src/InstructionDecoderImpl.C
  )

add_library (instructionAPI ${SRC_LIST})
target_link_libraries (instructionAPI common)
FILE (GLOB headers "h/*.h")
set_target_properties (instructionAPI PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS instructionAPI
        EXPORT dyninst
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
