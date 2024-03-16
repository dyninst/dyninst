include_guard(GLOBAL)

include(DyninstLibrary)

set(SRC_LIST
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
    src/dominator.C
    src/LoopAnalyzer.C
    src/Loop.C
    src/LoopTreeNode.C
    src/IdiomModelDesc.C
    src/ProbabilisticParser.C)

set(DATAFLOW_SRC
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
    ${PROJECT_SOURCE_DIR}/dataflowAPI/src/Visitors.C)

set(ROSE_SRC
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
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/DispatcherAMDGPU.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/DispatcherPowerpc.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/RegisterParts.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/Registers.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/BinarySymbolicExpr.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/RegisterStateGeneric.C
    ${PROJECT_SOURCE_DIR}/dataflowAPI/rose/semantics/SymEvalSemantics.C)

# FIXME: Rose needs a bunch of warning cleanup
set_source_files_properties(${ROSE_SRC} PROPERTIES LANGUAGE CXX COMPILE_FLAGS -w)
set(SRC_LIST ${SRC_LIST} ${DATAFLOW_SRC} ${ROSE_SRC})

if(LIGHTWEIGHT_SYMTAB)
  list(APPEND SRC_LIST src/SymLiteCodeSource.C)
else()
  list(APPEND SRC_LIST src/SymtabCodeSource.C)
endif()

file(GLOB parseapi_headers "h/*.h")
file(GLOB dataflowapi_headers "${PROJECT_SOURCE_DIR}/dataflowAPI/h/*.h")

if(ENABLE_PARSE_API_GRAPHS)
  list(APPEND SRC_LIST src/GraphAdapter.C)
endif()

# cmake-format: off
dyninst_library(
  parseAPI
  PRIVATE_HEADER_FILES ${parseapi_headers} ${dataflowapi_headers} 
  SOURCE_FILES ${SRC_LIST} ${ROSE_SRC}
  DEFINES PARSER_LIB DATAFLOW_LIB
  DYNINST_DEPS common instructionAPI ${SYMREADER}
  PUBLIC_DEPS Dyninst::Boost_headers
  PRIVATE_DEPS OpenMP::OpenMP_CXX
)
# cmake-format: on

foreach(t ${parseAPI_TARGETS})
  # There is both a common/h/util.h and a parseAPI/h/util.h, so put
  # the common/h include path first
  target_include_directories(${t} BEFORE
                             PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common/h>")

  target_include_directories(
    ${t} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dataflowAPI/h>")

  set_property(TARGET ${t} PROPERTY PUBLIC_HEADER ${parseapi_headers}
                                    ${dataflowapi_headers})

  if(DYNINST_OS_Windows)
    target_compile_definitions(${t} PRIVATE "ROSE_UTIL_EXPORTS" "NOMINMAX")
    target_link_libraries(parseAPI PRIVATE shlwapi)
  endif()

  install(TARGETS ${t} PUBLIC_HEADER DESTINATION ${DYNINST_INSTALL_INCLUDEDIR})

  # When using symlite, we still need the symtabAPI headers
  if(LIGHTWEIGHT_SYMTAB)
    target_include_directories(
      ${t} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/symtabAPI/h>")
  endif()

endforeach()
