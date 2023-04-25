include_guard(GLOBAL)

include(DyninstLibrary)

set(_public_headers
    h/AddrSpace.h
    h/CFGMaker.h
    h/Command.h
    h/Instrumenter.h
    h/PatchCallback.h
    h/PatchCFG.h
    h/PatchCommon.h
    h/PatchMgr.h
    h/PatchModifier.h
    h/PatchObject.h
    h/Point.h
    h/Snippet.h)

set(_private_headers src/ParseCallback.h)

set(_sources
    src/AddrSpace.C
    src/CFGMaker.C
    src/Command.C
    src/Instrumenter.C
    src/ParseCallback.C
    src/PatchBlock.C
    src/PatchCallback.C
    src/PatchEdge.C
    src/PatchFunction.C
    src/PatchLoop.C
    src/PatchLoopTreeNode.C
    src/PatchMgr.C
    src/PatchModifier.C
    src/PatchObject.C
    src/Point.C
    src/PointMaker.C)

# cmake-format: off
dyninst_library(
  patchAPI
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DEFINES PATCHAPI_LIB
  DYNINST_DEPS common instructionAPI parseAPI
  PRIVATE_DEPS Dyninst::Boost
)
# cmake-format: on
