# CMake configuration for patchAPI directory

include_directories (
    ${PROJECT_SOURCE_DIR}/patchAPI/src
  )

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
  )

add_library (patchAPI ${SRC_LIST})
target_link_libraries(patchAPI common)
target_link_libraries(patchAPI symtabAPI)
target_link_libraries(patchAPI instructionAPI)
target_link_libraries(patchAPI parseAPI)


INSTALL (DIRECTORY h/ DESTINATION include FILES_MATCHING PATTERN "*.h")
