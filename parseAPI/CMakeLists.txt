# CMake configuration for parseAPI directory

include_directories (
    ${PROJECT_SOURCE_DIR}/parseAPI/src
  )


set (SRC_LIST
        src/ParserDetails.C 
        src/Parser.C 
        src/CFGFactory.C 
        src/Function.C 
        src/Block.C 
        src/CodeObject.C 
        src/debug_parse.C 
        src/CodeSource.C 
        src/ParseData.C
        src/InstructionAdapter.C
        src/Parser-speculative.C
        src/ParseCallback.C 
        src/IA_x86Details.C 
        src/IA_powerDetails.C 
        src/IA_platformDetailsFactory.C 
        src/CFGModifier.C
        src/SymLiteCodeSource.C
        src/StackTamperVisitor.C
        ../dataflowAPI/src/ABI.C 
        ../dataflowAPI/src/Absloc.C 
        ../dataflowAPI/src/AbslocInterface.C 
        ../dataflowAPI/src/convertOpcodes.C 
        ../dataflowAPI/src/debug_dataflow.C 
        ../dataflowAPI/src/ExpressionConversionVisitor.C 
        ../dataflowAPI/src/InstructionCache.C 
        ../dataflowAPI/src/liveness.C 
        ../dataflowAPI/src/RegisterMap.C 
        ../dataflowAPI/src/RoseImpl.C 
        ../dataflowAPI/src/RoseInsnFactory.C 
        ../dataflowAPI/src/slicing.C 
        ../dataflowAPI/src/stackanalysis.C 
        ../dataflowAPI/src/SymbolicExpansion.C 
        ../dataflowAPI/src/SymEval.C 
        ../dataflowAPI/src/SymEvalPolicy.C 
        ../dataflowAPI/src/templates.C 
        ../dataflowAPI/src/Visitors.C 
    )

if (ENABLE_PARSE_API_GRAPHS)
set (SRC_LIST ${SRC_LIST}
     src/GraphAdapter.C
    )
endif()

if (NOT WITHOUT_SYMTAB_API)
set (SRC_LIST ${SRC_LIST}
    src/SymtabCodeSource.C
    )
endif()


add_library (parseAPI ${SRC_LIST})
