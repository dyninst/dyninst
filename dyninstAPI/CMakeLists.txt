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
     src/frameChecker.C
     src/BPatch_memoryAccessAdapter.C
)

# This is just .. messy. Sorry.


if (PLATFORM MATCHES i386 OR PLATFORM MATCHES amd64 OR PLATFORM MATCHES x86_64)
set (SRC_LIST ${SRC_LIST}
     src/RegisterConversion-x86.C
     src/Relocation/Widgets/CFWidget-x86.C
     src/inst-x86.C
     src/emit-x86.C
     src/codegen-x86.C
     src/stackwalk-x86.C
     src/dynProcess-x86.C
     src/parse-x86.C 
     src/IAPI_to_AST.C 
)
elseif (PLATFORM MATCHES ppc)
set (SRC_LIST ${SRC_LIST}
     src/inst-power.C 
     src/codegen-power.C 
     src/parse-power.C 
     src/RegisterConversion-ppc.C 
     src/stackwalk-ppc.C
     src/Relocation/Widgets/CFWidget-ppc.C
)
endif ()

if (PLATFORM MATCHES freebsd)
set (SRC_LIST ${SRC_LIST} 
     src/freebsd.C
     src/unix.C
     src/freebsd-x86.C
     src/syscallNotification.C 
     src/syscall-freebsd.C
)
elseif (PLATFORM MATCHES linux OR PLATFORM MATCHES bgq)
set (SRC_LIST ${SRC_LIST}
     src/linux.C
     src/inst-linux.C
     src/unix.C
     src/syscallNotification.C 
     src/syscall-linux.C
)
  if (PLATFORM MATCHES i386 OR PLATFORM MATCHES x86_64)
    set (SRC_LIST ${SRC_LIST} src/linux-x86.C)
  elseif (PLATFORM MATCHES ppc)
    set (SRC_LIST ${SRC_LIST} src/linux-power.C)
  endif()
elseif(PLATFORM MATCHES windows OR PLATFORM MATCHES nt)
    set (SRC_LIST ${SRC_LIST}
		src/hybridCallbacks.C
		src/hybridInstrumentation.C
		src/hybridOverwrites.C
        src/inst-winnt.C
        src/pdwinnt.C
        src/syscall-nt.C
		src/MemoryEmulator/memEmulator.C
		src/MemoryEmulator/memEmulatorTransformer.C
		src/MemoryEmulator/memEmulatorWidget.C
		src/Relocation/DynAddrSpace.C
		src/Relocation/DynCFGMaker.C
		src/Relocation/DynInstrumenter.C
		src/Relocation/DynObject.C
		src/Relocation/DynPointMaker.C
	)
endif()

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)
add_definitions(-DBPATCH_DLL_BUILD)
if (PLATFORM MATCHES i386 AND UNIX)
set (SRC_LIST ${SRC_LIST} src/cpuid-x86.S)
set_source_files_properties(src/cpuid-x86.S PROPERTIES LANGUAGE C)
endif ()

add_library (dyninstAPI ${SRC_LIST})
add_library (dyninstAPI_static STATIC ${SRC_LIST})


target_link_private_libraries (dyninstAPI common)
target_link_private_libraries (dyninstAPI instructionAPI)
target_link_private_libraries (dyninstAPI stackwalk)
target_link_private_libraries (dyninstAPI pcontrol)
target_link_private_libraries (dyninstAPI patchAPI)
target_link_private_libraries (dyninstAPI parseAPI)
target_link_private_libraries (dyninstAPI symtabAPI)

if (UNIX)
# Boost auto-links on Windows; don't double-link
target_link_private_libraries(dyninstAPI ${Boost_LIBRARIES})
target_link_private_libraries (dyninstAPI pthread)
endif()

FILE (GLOB headers "h/*.h")
set_target_properties (dyninstAPI dyninstAPI_static PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS dyninstAPI dyninstAPI_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
