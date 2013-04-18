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

FILE (GLOB headers "h/*.h")
set_target_properties (patchAPI PROPERTIES PUBLIC_HEADER "${headers}")


INSTALL (TARGETS patchAPI
        EXPORT dyninst
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
