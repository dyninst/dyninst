include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers
    CFG/RelocBlock.h
    CFG/RelocEdge.h
    CFG/RelocGraph.h
    CFG/RelocTarget.h
    CodeBuffer.h
    CodeMover.h
    CodeTracker.h
    DynAddrSpace.h
    DynCommon.h
    DynInstrumenter.h
    DynObject.h
    DynPointMaker.h
    Relocation.h
    Springboard.h
    Transformers/Include.h
    Transformers/Instrumenter.h
    Transformers/Modification.h
    Transformers/Movement-adhoc.h
    Transformers/Movement-analysis.h
    Transformers/Transformer.h
    Widgets/CallbackWidget.h
    Widgets/CFWidget.h
    Widgets/Include.h
    Widgets/InsnWidget.h
    Widgets/InstWidget.h
    Widgets/PCWidget.h
    Widgets/RelDataWidget.h
    Widgets/StackModWidget.h
    Widgets/Widget.h)

set(_sources
    CFG/RelocBlock.C
    CFG/RelocEdge.C
    CFG/RelocGraph.C
    CFG/RelocTarget.C
    CodeBuffer.C
    CodeMover.C
    CodeTracker.C
    DynAddrSpace.C
    DynCFGMaker.C
    DynInstrumenter.C
    DynObject.C
    DynPointMaker.C
    Springboard.C
    Transformers/Instrumenter.C
    Transformers/Modification.C
    Transformers/Movement-adhoc.C
    Transformers/Movement-analysis.C
    Transformers/Transformer.C
    Widgets/CFWidget.C
    Widgets/CallbackWidget.C
    Widgets/InsnWidget.C
    Widgets/InstWidget.C
    Widgets/PCWidget.C
    Widgets/RelDataWidget.C
    Widgets/StackModWidget.C
    patchapi_debug.C)

if(DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64)
  list(APPEND _sources Widgets/CFWidget-x86.C Widgets/PCWidget-x86.C)
elseif(DYNINST_CODEGEN_ARCH_PPC64LE)
  list(APPEND _sources Widgets/CFWidget-ppc.C Widgets/PCWidget-ppc.C)
elseif(DYNINST_CODEGEN_ARCH_AARCH64)
  list(APPEND _sources Widgets/CFWidget-aarch64.C Widgets/PCWidget-aarch64.C)
elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
  list(APPEND _sources Widgets/CFWidget-amdgpu.C Widgets/PCWidget-amdgpu.C)
endif()

# cmake-format: off
dyninst_library(
  dyninstAPI_relocation
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common parseAPI patchAPI pcontrol
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../ASTs
  ${CMAKE_CURRENT_SOURCE_DIR}/../BPatch
  ${CMAKE_CURRENT_SOURCE_DIR}/../registerSpace
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)
# cmake-format: on

target_include_directories(dyninstAPI_relocation PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
