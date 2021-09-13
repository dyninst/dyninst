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
    src/IA_x86.C
    src/IA_power.C
    src/IA_aarch64.C
    src/IA_amdgpu.C
    src/CFGModifier.C
    src/StackTamperVisitor.C
    src/JumpTableFormatPred.C
    src/JumpTableIndexPred.C
    src/IndirectAnalyzer.C
    src/IndirectASTVisitor.C
    src/SymbolicExpression.C
    src/BoundFactCalculator.C
    src/BoundFactData.C
    src/ThunkData.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/ABI.C
    src/dominator.C
    src/LoopAnalyzer.C
    src/Loop.C
    src/LoopTreeNode.C
    src/IdiomModelDesc.C
    src/ProbabilisticParser.C
)

set(ROSE_SRC
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/ABI.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/Absloc.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/AbslocInterface.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/convertOpcodes.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/debug_dataflow.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/ExpressionConversionVisitor.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/InstructionCache.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/liveness.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/RegisterMap.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/RoseImpl.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/RoseInsnFactory.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/slicing.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/stackanalysis.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/SymbolicExpansion.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/SymEval.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/SymEvalPolicy.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/templates.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/Visitors.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/ExtentMap.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/rangemap.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/Assert.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/Message.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/Sawyer.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/Synchronization.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/rose_getline.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/SmallObject.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/Stopwatch.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/StringUtility.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/Attribute.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/Combinatorics.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/util/LinearCongruentialGenerator.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/BaseSemantics2.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/DispatcherARM64.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/DispatcherAmdgpuVega.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/DispatcherPowerpc.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/RegisterParts.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/Registers.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/BinarySymbolicExpr.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/RegisterStateGeneric.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/SymEvalSemantics.C
)

# FIXME: Rose needs a bunch of warning cleanup
SET_SOURCE_FILES_PROPERTIES(${ROSE_SRC} PROPERTIES LANGUAGE CXX COMPILE_FLAGS -w)
set (SRC_LIST ${SRC_LIST}
	${ROSE_SRC}
)

if (LIGHTWEIGHT_SYMTAB)
    list(APPEND SRC_LIST src/SymLiteCodeSource.C)
else()
    list(APPEND SRC_LIST src/SymtabCodeSource.C)
endif()

if (ENABLE_PARSE_API_GRAPHS)
    list(APPEND SRC_LIST src/GraphAdapter.C)
endif()

set(_PRIVATE_COMPILE_DEFS)

if(WIN32)
    list(APPEND _PRIVATE_COMPILE_DEFS ROSE_UTIL_EXPORTS NOMINMAX)
endif()

file(GLOB HDR_LIST "${CMAKE_CURRENT_LIST_DIR}/h/*.h" "${PROJECT_SOURCE_DIR}/dataflowAPI/h/*.h")

dyninst_library(parseAPI
    HEADERS
        ${HDR_LIST}
    SOURCES
        ${SRC_LIST} ${ROSE_SRC}
    COMPILE_DEFINITIONS
        PRIVATE
            PARSER_LIB
            DATAFLOW_LIB
            ${_PRIVATE_COMPILE_DEFS}
    INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dataflowAPI/h>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    LINK_LIBRARIES
        Dyninst::common
        Dyninst::instructionAPI
        Dyninst::${SYMREADER}
        PRIVATE
            Dyninst::dynWinShlwapi
)

dyninst_message(STATUS "Architecture is: ${CMAKE_LIBRARY_ARCHITECTURE}")
