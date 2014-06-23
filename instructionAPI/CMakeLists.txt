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
SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

ADD_DEFINITIONS(-DINSTRUCTION_LIB)

add_library (instructionAPI ${SRC_LIST})
add_library (instructionAPI_static STATIC ${SRC_LIST})
target_link_private_libraries (instructionAPI common)
FILE (GLOB headers "h/*.h")
set_target_properties (instructionAPI instructionAPI_static PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS instructionAPI instructionAPI_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
