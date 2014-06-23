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
        src/IA_IAPI.C
        src/IA_x86Details.C 
        src/IA_powerDetails.C 
        src/IA_platformDetailsFactory.C 
        src/CFGModifier.C
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

if (LIGHTWEIGHT_SYMTAB)
set (SRC_LIST ${SRC_LIST}
    src/SymLiteCodeSource.C
)
else()
set (SRC_LIST ${SRC_LIST}
    src/SymtabCodeSource.C
)
endif()

if (PLATFORM MATCHES amd64 OR PLATFORM MATCHES i386 OR PLATFORM
MATCHES x86_64)
set (SRC_LIST ${SRC_LIST}
     src/InstructionSource-x86.C
     src/IA_x86.C
)
elseif (PLATFORM MATCHES ppc)
set (SRC_LIST ${SRC_LIST}
     src/InstructionSource-power.C
     src/IA_power.C
)
endif()

if (ENABLE_PARSE_API_GRAPHS)
set (SRC_LIST ${SRC_LIST}
     src/GraphAdapter.C
    )
endif()

SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

ADD_DEFINITIONS(-DPARSER_LIB)
ADD_DEFINITIONS(-DDATAFLOW_LIB)

add_library (parseAPI ${SRC_LIST})
add_library (parseAPI_static STATIC ${SRC_LIST})
target_link_private_libraries(parseAPI common)
target_link_private_libraries(parseAPI instructionAPI)

if (LIGHTWEIGHT_SYMTAB)
target_link_private_libraries(parseAPI symLite)
else()
target_link_private_libraries(parseAPI symtabAPI)
endif()

FILE (GLOB headers "h/*.h")
FILE (GLOB dataflowheaders "../dataflowAPI/h/*.h")
set_target_properties (parseAPI parseAPI_static PROPERTIES PUBLIC_HEADER "${headers};${dataflowheaders}")


INSTALL (TARGETS parseAPI parseAPI_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
