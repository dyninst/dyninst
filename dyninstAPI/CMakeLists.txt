include_guard(GLOBAL)

# dyninstAPI doesn't work with symlite
if(LIGHTWEIGHT_SYMTAB)
  message(STATUS "LIGHTWEIGHT_SYMTAB enabled; dyninstAPI not built.")
  return()
endif()

include(DyninstLibrary)

set(_public_headers
    h/BPatch_addressSpace.h
    h/BPatch_basicBlock.h
    h/BPatch_basicBlockLoop.h
    h/BPatch_binaryEdit.h
    h/BPatch_callbacks.h
    h/BPatch_dll.h
    h/BPatch_edge.h
    h/BPatch_enums.h
    h/BPatch_flowGraph.h
    h/BPatch_frame.h
    h/BPatch_function.h
    h/BPatch.h
    h/BPatch_image.h
    h/BPatch_instruction.h
    h/BPatch_loopTreeNode.h
    h/BPatch_memoryAccess_NP.h
    h/BPatch_module.h
    h/BPatch_object.h
    h/BPatch_parRegion.h
    h/BPatch_point.h
    h/BPatch_process.h
    h/BPatch_Set.h
    h/BPatch_snippet.h
    h/BPatch_sourceBlock.h
    h/BPatch_sourceObj.h
    h/BPatch_statement.h
    h/BPatch_thread.h
    h/BPatch_type.h
    h/BPatch_Vector.h
    h/StackMod.h)

set(_private_headers
    src/addressSpace.h
    src/arch-forward-decl.h
    src/ast.h
    src/baseTramp.h
    src/binaryEdit.h
    src/block.h
    src/BPatch_collections.h
    src/BPatch_libInfo.h
    src/BPatch_memoryAccessAdapter.h
    src/BPatch_private.h
    src/codegen-aarch64.h
    src/codegen.h
    src/codegen-power.h
    src/codegen-x86.h
    src/codeRange.h
    src/debug.h
    src/dynProcess.h
    src/dynThread.h
    src/ELF_Section.h
    src/emit-aarch64.h
    src/emit-power.h
    src/emitter.h
    src/emit-x86.h
    src/frameChecker.h
    src/frame.h
    src/freebsd.h
    src/freebsd-x86.h
    src/function.h
    src/hybridAnalysis.h
    src/IAPI_to_AST.h
    src/image.h
    src/infHeap.h
    src/inst-aarch64.h
    src/inst.h
    src/instP.h
    src/instPoint.h
    src/inst-power.h
    src/inst-x86.h
    src/legacy-instruction.h
    src/LinearVariable.h
    src/linux-aarch64.h
    src/linux.h
    src/linux-power.h
    src/linux-x86.h
    src/mapped_module.h
    src/mapped_object.h
    src/nt_signal_emul.h
    src/opcode.h
    src/os.h
    src/parRegion.h
    src/parse-cfg.h
    src/Parsing.h
    src/patch.h
    src/Patching.h
    src/pcEventHandler.h
    src/pcEventMuxer.h
    src/pcrel.h
    src/pdwinnt.h
    src/RegisterConversion.h
    src/registerSpace.h
    src/syscallNotification.h
    src/syscalltrap.h
    src/trapMappings.h
    src/unix.h
    src/util.h)

set(_sources
    src/addressSpace.C
    src/ast.C
    src/baseTramp.C
    src/binaryEdit.C
    src/block.C
    src/codeRange.C
    src/codegen.C
    src/debug.C
    src/dynProcess.C
    src/dynThread.C
    src/edge.C
    src/frame.C
    src/frameChecker.C
    src/function.C
    src/hybridCallbacks.C
    src/hybridInstrumentation.C
    src/hybridOverwrites.C
    src/image.C
    src/infHeap.C
    src/inst.C
    src/instPoint.C
    src/mapped_module.C
    src/mapped_object.C
    src/parRegion.C
    src/parse-cfg.C
    src/pcEventHandler.C
    src/pcEventMuxer.C
    src/registerSpace.C
    src/util.C
    src/variable.C
    src/BPatch.C
    src/BPatch_addressSpace.C
    src/BPatch_basicBlock.C
    src/BPatch_basicBlockLoop.C
    src/BPatch_binaryEdit.C
    src/BPatch_collections.C
    src/BPatch_edge.C
    src/BPatch_flowGraph.C
    src/BPatch_frame.C
    src/BPatch_function.C
    src/BPatch_image.C
    src/BPatch_instruction.C
    src/BPatch_loopTreeNode.C
    src/BPatch_memoryAccess.C
    src/BPatch_memoryAccessAdapter.C
    src/BPatch_module.C
    src/BPatch_object.C
    src/BPatch_parRegion.C
    src/BPatch_point.C
    src/BPatch_process.C
    src/BPatch_snippet.C
    src/BPatch_sourceBlock.C
    src/BPatch_statement.C
    src/BPatch_thread.C
    src/BPatch_type.C
    src/Parsing-arch.C
    src/Parsing.C
    src/Patching.C
    src/Relocation/CFG/RelocBlock.C
    src/Relocation/CFG/RelocEdge.C
    src/Relocation/CFG/RelocGraph.C
    src/Relocation/CFG/RelocTarget.C
    src/Relocation/CodeBuffer.C
    src/Relocation/CodeMover.C
    src/Relocation/CodeTracker.C
    src/Relocation/DynAddrSpace.C
    src/Relocation/DynCFGMaker.C
    src/Relocation/DynInstrumenter.C
    src/Relocation/DynObject.C
    src/Relocation/DynPointMaker.C
    src/Relocation/Springboard.C
    src/Relocation/Transformers/Instrumenter.C
    src/Relocation/Transformers/Modification.C
    src/Relocation/Transformers/Movement-adhoc.C
    src/Relocation/Transformers/Movement-analysis.C
    src/Relocation/Transformers/Transformer.C
    src/Relocation/Widgets/ASTWidget.C
    src/Relocation/Widgets/CFWidget.C
    src/Relocation/Widgets/CallbackWidget.C
    src/Relocation/Widgets/InsnWidget.C
    src/Relocation/Widgets/InstWidget.C
    src/Relocation/Widgets/PCWidget.C
    src/Relocation/Widgets/RelDataWidget.C
    src/Relocation/Widgets/StackModWidget.C
    src/Relocation/patchapi_debug.C)

list(FIND DYNINST_PLATFORM_CAPABILITIES "-Dcap_stack_mods" cap_stack_mods_found)
if(cap_stack_mods_found GREATER -1)
  list(
    APPEND
    _sources
    src/StackMod/OffsetVector.C
    src/StackMod/StackAccess.C
    src/StackMod/StackLocation.C
    src/StackMod/StackMod.C
    src/StackMod/StackModExpr.C
    src/StackMod/StackModChecker.C
    src/StackMod/TMap.C)
endif()

if(DYNINST_ARCH_i386 OR DYNINST_ARCH_x86_64)
  list(
    APPEND
    _sources
    src/RegisterConversion-x86.C
    src/Relocation/Widgets/CFWidget-x86.C
    src/Relocation/Widgets/PCWidget-x86.C
    src/inst-x86.C
    src/emit-x86.C
    src/codegen-x86.C
    src/stackwalk-x86.C
    src/dynProcess-x86.C
    src/parse-x86.C
    src/IAPI_to_AST.C)
elseif(DYNINST_ARCH_ppc64le)
  list(
    APPEND
    _sources
    src/inst-power.C
    src/codegen-power.C
    src/parse-power.C
    src/RegisterConversion-ppc.C
    src/stackwalk-ppc.C
    src/Relocation/Widgets/CFWidget-ppc.C
    src/Relocation/Widgets/PCWidget-ppc.C)
elseif(DYNINST_ARCH_aarch64)
  list(
    APPEND
    _sources
    src/inst-aarch64.C
    src/emit-aarch64.C
    src/codegen-aarch64.C
    src/parse-aarch64.C
    src/RegisterConversion-aarch64.C
    src/stackwalk-aarch64.C
    src/Relocation/Widgets/CFWidget-aarch64.C
    src/Relocation/Widgets/PCWidget-aarch64.C)
endif()

if(DYNINST_OS_FreeBSD)
  list(
    APPEND
    _sources
    src/freebsd.C
    src/unix.C
    src/freebsd-x86.C
    src/syscallNotification.C
    src/syscall-freebsd.C)
elseif(DYNINST_OS_Linux)
  list(
    APPEND
    _sources
    src/linux.C
    src/inst-linux.C
    src/unix.C
    src/syscallNotification.C
    src/syscall-linux.C)
  if(DYNINST_ARCH_i386 OR DYNINST_ARCH_x86_64)
    list(APPEND _sources src/linux-x86.C)
  elseif(DYNINST_ARCH_ppc64le)
    list(APPEND _sources src/linux-power.C)
  elseif(DYNINST_ARCH_aarch64)
    list(APPEND _sources src/linux-aarch64.C)
  endif()
elseif(DYNINST_OS_Windows)
  list(
    APPEND
    _sources
    src/hybridCallbacks.C
    src/hybridInstrumentation.C
    src/hybridOverwrites.C
    src/inst-winnt.C
    src/pdwinnt.C
    src/syscall-nt.C
    src/Relocation/DynAddrSpace.C
    src/Relocation/DynCFGMaker.C
    src/Relocation/DynInstrumenter.C
    src/Relocation/DynObject.C
    src/Relocation/DynPointMaker.C)
endif()

if(DYNINST_ARCH_i386 AND DYNINST_OS_UNIX)
  list(APPEND _sources src/cpuid-x86.S)
  set_source_files_properties(src/cpuid-x86.S PROPERTIES LANGUAGE C)
endif()

# cmake-format: off
dyninst_library(
  dyninstAPI
  PUBLIC_HEADER_FILES ${_public_headers}
  SOURCE_FILES ${_sources}
  DEFINES BPATCH_DLL_BUILD
  DYNINST_DEPS common instructionAPI stackwalk pcontrol patchAPI parseAPI symtabAPI 
  PUBLIC_DEPS Dyninst::Boost_headers
  PRIVATE_DEPS Dyninst::ElfUtils Threads::Threads
)
# cmake-format: on

# gcc/clang search directories
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-search-dirs
                  OUTPUT_VARIABLE SEARCH_DIRS)
  if(SEARCH_DIRS MATCHES "libraries:[ ]?[=]?(.+)\n")
    set(DYNINST_COMPILER_SEARCH_DIRS ${CMAKE_MATCH_1})
  endif()
endif()

foreach(t ${dyninstAPI_TARGETS})
  if(DYNINST_OS_Windows)
    target_link_libraries(${t} PRIVATE dbghelp WS2_32 imagehlp)
  endif()

  if(DYNINST_OS_UNIX)
    target_link_libraries(${t} PRIVATE pthread)
    target_compile_definitions(
      ${t} PRIVATE "DYNINST_COMPILER_SEARCH_DIRS=${DYNINST_COMPILER_SEARCH_DIRS}")
  endif()
endforeach()
