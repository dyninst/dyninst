# CMake configuration for patchAPI directory

set (SRC_LIST
    src/AddrSpace.C
    src/Instrumenter.C
    src/PatchObject.C
    src/PatchBlock.C
    src/PatchEdge.C
    src/PatchFunction.C
    src/PatchMgr.C
    src/Point.C
    src/CFGMaker.C
    src/PointMaker.C
    src/Command.C
    src/PatchCallback.C
    src/ParseCallback.C
    src/PatchModifier.C
    src/PatchLoop.C
    src/PatchLoopTreeNode.C
)

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)

dyninst_library(patchAPI
    SOURCES
        ${SRC_LIST}
    COMPILE_DEFINITIONS
        PRIVATE PATCHAPI_LIB
    LINK_LIBRARIES
        Dyninst::headers
        Dyninst::common
        Dyninst::instructionAPI
        Dyninst::parseAPI
)
