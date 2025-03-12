include_guard(GLOBAL)

include(DyninstLibrary)
include(${PROJECT_SOURCE_DIR}/dataflowAPI/CMakeLists.txt)

set(_parseapi_sources
    src/Block.C
    src/BoundFactCalculator.C
    src/BoundFactData.C
    src/CFGFactory.C
    src/CFGModifier.C
    src/CodeObject.C
    src/CodeSource.C
    src/debug_parse.C
    src/dominator.C
    src/Function.C
    src/IA_aarch64.C
    src/IA_amdgpu.C
    src/IA_IAPI.C
    src/IA_power.C
    src/IA_x86.C
    src/IdiomModelDesc.C
    src/IndirectAnalyzer.C
    src/IndirectASTVisitor.C
    src/InstructionAdapter.C
    src/JumpTableFormatPred.C
    src/JumpTableIndexPred.C
    src/LoopAnalyzer.C
    src/Loop.C
    src/LoopTreeNode.C
    src/ParseCallback.C
    src/ParseData.C
    src/Parser.C
    src/ParserDetails.C
    src/Parser-speculative.C
    src/ProbabilisticParser.C
    src/StackTamperVisitor.C
    src/SymbolicExpression.C
    src/SymtabCodeSource.C
    src/ThunkData.C)

set(_parseapi_public_headers
    h/CFGFactory.h
    h/CFG.h
    h/CFGModifier.h
    h/CodeObject.h
    h/CodeSource.h
    h/InstructionAdapter.h
    h/InstructionSource.h
    h/Location.h
    h/LockFreeQueue.h
    h/ParseCallback.h
    h/ParseContainers.h)

set(_parseapi_private_headers
    src/BoundFactCalculator.h
    src/BoundFactData.h
    src/debug_parse.h
    src/dominator.h
    src/IA_aarch64.h
    src/IA_amdgpu.h
    src/IA_IAPI.h
    src/IA_power.h
    src/IA_x86.h
    src/IndirectAnalyzer.h
    src/IndirectASTVisitor.h
    src/JumpTableFormatPred.h
    src/JumpTableIndexPred.h
    src/LoopAnalyzer.h
    src/ParseData.h
    src/ParserDetails.h
    src/Parser.h
    src/ProbabilisticParser.h
    src/StackTamperVisitor.h
    src/SymbolicExpression.h
    src/ThunkData.h
    src/util.h)

if(LIGHTWEIGHT_SYMTAB)
  list(APPEND _parseapi_sources src/SymLiteCodeSource.C)
  list(APPEND _parseapi_public_headers h/SymLiteCodeSource.h)
else()
  list(APPEND _parseapi_sources src/SymtabCodeSource.C)
endif()

if(ENABLE_PARSE_API_GRAPHS)
  list(APPEND _parseapi_sources src/GraphAdapter.C)
endif()

# cmake-format: off
dyninst_library(
  parseAPI
  PRIVATE_HEADER_FILES ${_parseapi_private_headers} ${_dataflow_private_headers} ${_isa_spec_private_headers}
  PUBLIC_HEADER_FILES ${_parseapi_public_headers} ${_dataflow_public_headers} 
  SOURCE_FILES ${_parseapi_sources} ${_dataflow_sources} ${_isa_spec_sources}
  DYNINST_DEPS common instructionAPI ${SYMREADER}
  PUBLIC_DEPS Dyninst::Boost_headers
  PRIVATE_DEPS OpenMP::OpenMP_CXX
)
# cmake-format: on

#find_package(IsaExplorer REQUIRED COMPONENTS headers CONFIG PATHS "${PROJECT_SOURCE_DIR}/isa_spec_manager/build/linux/output/libisa_explorer_dynamic.so")
#find_package(IsaDecoder REQUIRED COMPONENTS headers CONFIG PATHS "${PROJECT_SOURCE_DIR}/isa_spec_manager/build/linux/output/libisa_decoder_dynamic.so")

#target_link_libraries(parseAPI PRIVATE IsaExplorer IsaDecoder)

foreach(t ${parseAPI_TARGETS})
  # There is both a common/h/util.h and a parseAPI/h/util.h, so put
  # the common/h include path first
  target_include_directories(${t} BEFORE
                             PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common/h>")

  target_include_directories(
    ${t} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dataflowAPI/h>"
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dataflowAPI/>"
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/isa_spec_manager/include/>"
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/isa_spec_manager/source/isa_executor/generated_cpp/>"
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/isa_spec_manager/source/isa_executor/>"
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/isa_spec_manager/source/common>"
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/isa_spec_manager/source/third_party/tinyxml2>"
                )

  if(DYNINST_OS_Windows)
    target_compile_definitions(${t} PRIVATE "ROSE_UTIL_EXPORTS" "NOMINMAX")
    target_link_libraries(parseAPI PRIVATE shlwapi)
  endif()

  # When using symlite, we still need the symtabAPI headers
  if(LIGHTWEIGHT_SYMTAB)
    target_include_directories(
      ${t} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/symtabAPI/h>")
  endif()

endforeach()
