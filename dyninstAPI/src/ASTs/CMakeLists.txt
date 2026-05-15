include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers
    addressAST.h
    ast.h
    ast_helpers.h
    atomicOperationAST.h
    codeGenAST.h
    functionCallAST.h
    genericStackAST.h
    jumpTargetAST.h
    memoryAccessAST.h
    nullAST.h
    operandAST.h
    OperandType.h
    operatorAST.h
    scrambleRegistersAST.h
    sequenceAST.h
    snippetAST.h
    stackAST.h
    stackInsertionAST.h
    stackRemovalAST.h
    variableAST.h)

set(_sources
    actualAddressAST.C
    atomicOperationAST.C
    codeGenAST.C
    functionCallAST.C
    genericStackAST.C
    jumpTargetAST.C
    memoryAccessAST.C
    nullAST.C
    operandAST.C
    operatorAST.C
    originalAddressAST.C
    scrambleRegistersAST.C
    sequenceAST.C
    snippetAST.C
    stackAST.C
    stackInsertionAST.C
    stackRemovalAST.C
    variableAST.C)

if(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
  list(APPEND _headers AmdgpuEpilogue.h AmdgpuPrologue.h)
  list(APPEND _sources AmdgpuEpilogue.C AmdgpuPrologue.C)
endif()

add_library(dyninstAPI_ASTs INTERFACE)
target_sources(dyninstAPI_ASTs INTERFACE ${_headers} ${_sources})
target_include_directories(dyninstAPI_ASTs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

unset(_headers)
unset(_sources)
