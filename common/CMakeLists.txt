# CMake configuration for common directory

if(NOT DYNINST_OS_Windows)
    include_directories(${PROJECT_SOURCE_DIR}/common/h ${PROJECT_SOURCE_DIR}/common/src)
else()
    include_directories(
        ${PROJECT_SOURCE_DIR}/common/h # don't include common/src; anything from there can
                                       # still collide with default includes.
        # stupid Windows case-insensitive naming.
        )
endif()

set(SRC_LIST
    src/pfq-rwlock.C
    src/concurrent.C
    src/Timer.C
    src/Types.C
    src/lprintf.C
    src/pathName.C
    src/stats.C
    src/Annotatable.C
    src/MappedFile.C
    src/sha1.C
    src/util.C
    src/Node.C
    src/Graph.C
    src/Edge.C
    src/DOT.C
    src/dyn_regs.C
    src/AST.C
    src/addrtranslate.C
    src/arch-x86.C
    src/arch-power.C
    src/arch-aarch64.C
    src/debug_common.C
    src/VariableLocation.C
    src/Buffer.C
    src/MachSyscall.C)

set(C_SRC_LIST)

if(DYNINST_OS_FreeBSD)
    set(SRC_LIST ${SRC_LIST} src/freebsdKludges.C src/addrtranslate-sysv.C
                 src/addrtranslate-freebsd.C src/symbolDemangleWithCache.C)
    set(C_SRC_LIST ${C_SRC_LIST} src/symbolDemangle.c)
endif()

if(DYNINST_OS_Linux)
    set(SRC_LIST
        ${SRC_LIST}
        src/linuxKludges.C
        src/parseauxv.C
        src/addrtranslate-sysv.C
        src/addrtranslate-auxv.C
        src/addrtranslate-linux.C
        src/symbolDemangleWithCache.C)
    set(C_SRC_LIST ${C_SRC_LIST} src/symbolDemangle.c)
endif()

if(DYNINST_OS_Windows)
    set(SRC_LIST
        ${SRC_LIST} src/ntKludges.C
        # src/dthread-win.C src/dthread.C
        src/addrtranslate-win.C)
    add_definitions(-DWIN32 -D_WIN32_WINNT=0x500)
endif()

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${C_SRC_LIST} PROPERTIES LANGUAGE C)
set(SRC_LIST ${SRC_LIST} ${C_SRC_LIST})

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if(${CMAKE_CXX_COMPILER_VERSION} MATCHES "^[789](\.|$)")
        # Disable var-tracking-assignments for arch-x86.C for gcc 7, 8, & 9. The default
        # max size for these compilers is too small so it fails, adjusting it using
        #
        # PROPERTIES COMPILE_FLAGS "--param=max-vartrack-size=600000000"
        #
        # succeeds, but just disable it.
        set_source_files_properties(
            src/arch-x86.C PROPERTIES COMPILE_FLAGS "-fno-var-tracking-assignments")
    endif()
endif()

# adjust warning threshold if set in cmake/warnings.cmake
if(debugMaxFrameSizeOverrideSyscallInformation)
    set_source_files_properties(
        src/MachSyscall.C
        PROPERTIES
            COMPILE_FLAGS
            $<$<CONFIG:DEBUG>:-Wframe-larger-than=${debugMaxFrameSizeOverrideSyscallInformation}>
        )
endif()
if(nonDebugMaxFrameSizeOverrideSyscallInformation)
    set_source_files_properties(
        src/MachSyscall.C
        PROPERTIES
            COMPILE_FLAGS
            $<$<NOT:$<CONFIG:DEBUG>>:-Wframe-larger-than=${nonDebugMaxFrameSizeOverrideSyscallInformation}>
        )
endif()

add_definitions(-DCOMMON_LIB)

dyninst_library(common)

if(${ENABLE_STATIC_LIBS})
    target_link_libraries(common_static PRIVATE Dyninst::LibIberty)
endif()

if(DYNINST_OS_Windows)
    target_link_libraries(common PRIVATE Psapi WS2_32 dbghelp)
endif()

target_link_libraries(common PUBLIC Dyninst::TBB Dyninst::Boost)
target_link_libraries(common PRIVATE Dyninst::LibIberty OpenMP::OpenMP_CXX Dyninst::Valgrind)

# Install AMDGPU headers under subdirectory of common/h/AMDGPU
set(AMDGPU_HEADER ${PROJECT_SOURCE_DIR}/common/h/AMDGPU)
install(DIRECTORY ${AMDGPU_HEADER} DESTINATION ${INSTALL_INCLUDE_DIR})
