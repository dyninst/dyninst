include_guard(GLOBAL)

# dyninstAPI doesn't work with symlite
if(LIGHTWEIGHT_SYMTAB)
  message(STATUS "LIGHTWEIGHT_SYMTAB enabled; dyninstAPI not built.")
  return()
endif()

include(DyninstLibrary)

add_subdirectory(src/ASTs)
add_subdirectory(src/BPatch)
add_subdirectory(src/codegen)
add_subdirectory(src/parsing)
add_subdirectory(src/patching)
add_subdirectory(src/registerSpace)
add_subdirectory(src/Relocation)
add_subdirectory(src/StackMod)
add_subdirectory(src/trampolines)

set(_subprojects
    dyninstAPI_ASTs
    dyninstAPI_bpatch
    dyninstAPI_codegen
    dyninstAPI_parsing
    dyninstAPI_patching
    dyninstAPI_registerSpace
    dyninstAPI_relocation
    dyninstAPI_stackmod
    dyninstAPI_trampolines)

set(_public_headers ${_BPatch_public_headers})

set(_private_headers
    src/addressSpace.h
    src/AmdgpuKernelDescriptor.h
    src/AmdgpuPointHandler.h
    ${PROJECT_SOURCE_DIR}/external/amdgpu/AMDHSAKernelDescriptor.h
    ${PROJECT_SOURCE_DIR}/external/amdgpu/AMDGPUEFlags.h
    src/binaryEdit.h
    src/codegen-amdgpu.h
    src/codegen-aarch64.h
    src/codegen.h
    src/codegen-power.h
    src/codegen-x86.h
    src/codeRange.h
    src/debug.h
    src/dynProcess.h
    src/dynThread.h
    src/emit-amdgpu.h
    src/emit-power.h
    src/emitter.h
    src/emit-x86.h
    src/frameChecker.h
    src/frame.h
    src/freebsd.h
    src/function_cache.h
    src/hybridAnalysis.h
    src/IAPI_to_AST.h
    src/image.h
    src/infHeap.h
    src/inst-aarch64.h
    src/inst-power.h
    src/inst-x86.h
    src/linux-aarch64.h
    src/linux.h
    src/linux-power.h
    src/linux-x86.h
    src/mapped_module.h
    src/mapped_object.h
    src/nt_signal_emul.h
    src/opcode.h
    src/os.h
    src/pcEventHandler.h
    src/pcEventMuxer.h
    src/pdwinnt.h
    src/PointHandler.h
    src/RegisterConversion.h
    src/regTracker.h
    src/syscallNotification.h
    src/trapMappings.h
    src/unix.h
    src/util.h
    src/amdgpu-gfx908-details.h)

set(_sources
    src/addressSpace.C
    src/binaryEdit.C
    src/codeRange.C
    src/codegen.C
    src/debug.C
    src/dynProcess.C
    src/dynThread.C
    src/frame.C
    src/frameChecker.C
    src/hybridCallbacks.C
    src/hybridInstrumentation.C
    src/hybridOverwrites.C
    src/image.C
    src/infHeap.C
    src/mapped_module.C
    src/mapped_object.C
    src/pcEventHandler.C
    src/pcEventMuxer.C
    src/regTracker.C
    src/variable.C)

if(DYNINST_HOST_ARCH_I386 OR DYNINST_HOST_ARCH_X86_64)
  list(APPEND _sources src/stackwalk-x86.C src/dynProcess-x86.C)
elseif(DYNINST_HOST_ARCH_PPC64LE)
  list(APPEND _sources src/stackwalk-ppc.C src/dynProcess-power.C)
elseif(DYNINST_HOST_ARCH_AARCH64)
  list(APPEND _sources src/stackwalk-aarch64.C src/dynProcess-aarch64.C)
endif()

if(DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64)
  list(
    APPEND
    _sources
    src/RegisterConversion-x86.C
    src/inst-x86.C
    src/emit-x86.C
    src/codegen-x86.C
    src/IAPI_to_AST.C)
elseif(DYNINST_CODEGEN_ARCH_PPC64LE)
  list(APPEND _sources src/inst-power.C src/codegen-power.C src/RegisterConversion-ppc.C)
elseif(DYNINST_CODEGEN_ARCH_AARCH64)
  list(APPEND _sources src/inst-aarch64.C src/codegen-aarch64.C
       src/RegisterConversion-aarch64.C)
elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
  list(
    APPEND
    _sources
    src/inst-amdgpu.C
    src/emit-amdgpu.C
    src/codegen-amdgpu.C
    src/RegisterConversion-amdgpu.C
    src/AmdgpuKernelDescriptor.C
    src/AmdgpuPointHandler.C
    src/amdgpu-gfx908-details.C)
endif()

if(DYNINST_OS_FreeBSD)
  list(APPEND _sources src/freebsd.C src/unix.C src/syscallNotification.C
       src/syscall-freebsd.C)
elseif(DYNINST_OS_Linux)
  list(APPEND _sources src/linux.C src/unix.C src/syscallNotification.C
       src/syscall-linux.C)
  if(DYNINST_HOST_ARCH_I386 OR DYNINST_HOST_ARCH_X86_64)
    list(APPEND _sources src/linux-x86.C)
  elseif(DYNINST_HOST_ARCH_PPC64LE)
    list(APPEND _sources src/linux-power.C)
  elseif(DYNINST_HOST_ARCH_AARCH64)
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
    src/syscall-nt.C)
endif()

# cmake-format: off
dyninst_library(
  dyninstAPI
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DYNINST_DEPS common instructionAPI stackwalk pcontrol patchAPI parseAPI symtabAPI
  DYNINST_INTERNAL_DEPS dynDwarf dynElf ${_subprojects}
  PUBLIC_DEPS Dyninst::Boost_headers
  PRIVATE_DEPS Threads::Threads Dyninst::PeParse
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
    target_compile_definitions(
      ${t} PRIVATE "DYNINST_COMPILER_SEARCH_DIRS=${DYNINST_COMPILER_SEARCH_DIRS}")
  endif()

  foreach(sp ${_subprojects})
    target_include_directories(
      ${t}
      PUBLIC "$<BUILD_INTERFACE:$<TARGET_PROPERTY:${sp},INTERFACE_INCLUDE_DIRECTORIES>>")
  endforeach()

endforeach()

unset(_inc_dirs)
