include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_BPatch_public_headers
    h/BPatch_addressSpace.h
    h/BPatch_basicBlock.h
    h/BPatch_basicBlockLoop.h
    h/BPatch_binaryEdit.h
    h/BPatch_callbacks.h
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
    h/BPatch_point.h
    h/BPatch_process.h
    h/BPatch_snippet.h
    h/BPatch_sourceBlock.h
    h/BPatch_sourceObj.h
    h/BPatch_statement.h
    h/BPatch_thread.h
    h/BPatch_type.h
    h/BPatch_Vector.h
    h/StackMod.h
    PARENT_SCOPE)

set(_headers BPatch_collections.h BPatch_libInfo.h BPatch_memoryAccessAdapter.h
             BPatch_private.h)

set(_sources
    BPatch.C
    BPatch_addressSpace.C
    BPatch_basicBlock.C
    BPatch_basicBlockLoop.C
    BPatch_binaryEdit.C
    BPatch_collections.C
    BPatch_edge.C
    BPatch_flowGraph.C
    BPatch_frame.C
    BPatch_function.C
    BPatch_image.C
    BPatch_instruction.C
    BPatch_loopTreeNode.C
    BPatch_memoryAccess.C
    BPatch_memoryAccessAdapter.C
    BPatch_module.C
    BPatch_object.C
    BPatch_point.C
    BPatch_process.C
    BPatch_snippet.C
    BPatch_sourceBlock.C
    BPatch_statement.C
    BPatch_thread.C
    BPatch_type.C)

add_library(dyninstAPI_bpatch INTERFACE)
target_sources(dyninstAPI_bpatch INTERFACE ${_headers} ${_sources})
target_include_directories(dyninstAPI_bpatch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

unset(_headers)
unset(_sources)
