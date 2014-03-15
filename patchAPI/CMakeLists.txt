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

SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

ADD_DEFINITIONS(-DPATCHAPI_LIB)


add_library (patchAPI ${SRC_LIST})
add_library (patchAPI_static STATIC ${SRC_LIST})
target_link_private_libraries(patchAPI common)
target_link_private_libraries(patchAPI instructionAPI)
target_link_private_libraries(patchAPI parseAPI)

FILE (GLOB headers "h/*.h")
set_target_properties (patchAPI patchAPI_static PROPERTIES PUBLIC_HEADER "${headers}")


INSTALL (TARGETS patchAPI patchAPI_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
