# CMake configuration for dyninstAPI directory

include_directories (
    ${PROJECT_SOURCE_DIR}/dyninstAPI/src
  )

set (SRC_LIST
     src/BPatch.C 
     src/BPatch_image.C 
     src/BPatch_function.C 
     src/BPatch_snippet.C 
     src/BPatch_thread.C 
     src/BPatch_process.C 
     src/BPatch_type.C 
     src/BPatch_module.C 
     src/BPatch_object.C 
     src/BPatch_point.C 
     src/BPatch_collections.C 
     src/BPatch_sourceBlock.C 
     src/BPatch_basicBlock.C 
     src/BPatch_basicBlockLoop.C 
     src/BPatch_edge.C 
     src/BPatch_loopTreeNode.C 
     src/BPatch_flowGraph.C 
     src/BPatch_frame.C 
     src/BPatch_parRegion.C 
     src/BPatch_statement.C 
     src/BPatch_addressSpace.C 
     src/BPatch_binaryEdit.C 
     src/BPatch_memoryAccess.C 
     src/dummy.C 
     src/debug.C 
     src/ast.C 
     src/registerSpace.C 
     src/codegen.C 
     src/inst.C 
     src/instPoint.C 
     src/baseTramp.C 
     src/addressSpace.C 
     src/binaryEdit.C 
     src/infHeap.C 
     src/frame.C 
     src/codeRange.C 
     src/image.C 
     src/dominator.C 
     src/parse-cfg.C 
     src/mapped_object.C 
     src/mapped_module.C 
     src/function.C 
     src/block.C 
     src/edge.C 
     src/variable.C 
     src/util.C 
     src/BPatch_instruction.C 
     src/parRegion.C 
     src/Parsing.C 
     src/Parsing-arch.C 
     src/hybridInstrumentation.C 
     src/hybridOverwrites.C 
     src/hybridCallbacks.C 
     src/dynProcess.C 
     src/dynThread.C 
     src/pcEventHandler.C 
     src/pcEventMuxer.C 
     src/syscallNotification.C 
     src/Relocation/CodeMover.C 
     src/Relocation/CFG/RelocGraph.C 
     src/Relocation/CFG/RelocBlock.C 
     src/Relocation/CFG/RelocEdge.C 
     src/Relocation/CFG/RelocTarget.C 
     src/Relocation/Springboard.C 
     src/Relocation/Widgets/ASTWidget.C 
     src/Relocation/Widgets/CFWidget.C 
     src/Relocation/Widgets/CallbackWidget.C 
     src/Relocation/Widgets/InsnWidget.C 
     src/Relocation/Widgets/InstWidget.C 
     src/Relocation/Widgets/PCWidget.C 
     src/Relocation/Widgets/RelDataWidget.C 
     src/Relocation/Transformers/Transformer.C 
     src/Relocation/Transformers/Instrumenter.C 
     src/Relocation/Transformers/Modification.C 
     src/Relocation/Transformers/Movement-adhoc.C 
     src/Relocation/Transformers/Movement-analysis.C 
     src/Relocation/CodeTracker.C 
     src/Relocation/CodeBuffer.C 
     src/Relocation/patchapi_debug.C 
     src/Relocation/DynObject.C 
     src/Relocation/DynAddrSpace.C 
     src/Relocation/DynPointMaker.C 
     src/Relocation/DynCFGMaker.C 
     src/Relocation/DynInstrumenter.C 
     src/Patching.C
)

add_library (dyninstAPI ${SRC_LIST})

target_link_libraries (dyninstAPI pcontrol)

if (UNIX)
target_link_libraries (dyninstAPI ${LIBELF_LIBRARIES})
target_link_libraries (dyninstAPI ${LIBDWARF_LIBRARIES})
endif()

if (LIGHTWEIGHT_SYMTAB)
target_link_libraries(dyninstAPI symLite)
else()
target_link_libraries(dyninstAPI symtabAPI)
endif()

if (SW_ANALYSIS_STEPPER)
target_link_libraries(dyninstAPI parseAPI)
endif()

INSTALL (DIRECTORY h/ DESTINATION include FILES_MATCHING PATTERN "*.h")
