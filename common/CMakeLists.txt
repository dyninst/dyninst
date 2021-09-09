# CMake configuration for common directory

set (SRC_LIST
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
    src/MachSyscall.C
)

set (C_SRC_LIST)

if (PLATFORM MATCHES freebsd)
    set (SRC_LIST ${SRC_LIST}
        src/freebsdKludges.C
        src/addrtranslate-sysv.C
        src/addrtranslate-freebsd.C
        src/symbolDemangleWithCache.C
    )
    set (C_SRC_LIST ${C_SRC_LIST}
        src/symbolDemangle.c
    )
endif()

if (PLATFORM MATCHES linux)
    set (SRC_LIST ${SRC_LIST}
        src/linuxKludges.C
        src/parseauxv.C
        src/addrtranslate-sysv.C
        src/addrtranslate-auxv.C
        src/addrtranslate-linux.C
        src/symbolDemangleWithCache.C
    )
    set (C_SRC_LIST ${C_SRC_LIST}
        src/symbolDemangle.c
    )
endif()

if (PLATFORM MATCHES cnl)
    set (SRC_LIST ${SRC_LIST}
        src/linuxKludges.C
        src/parseauxv.C
        src/addrtranslate-sysv.C
        src/addrtranslate-auxv.C
        src/addrtranslate-linux.C
    )
endif()

set(_PRIVATE_COMPILE_DEFS)
if (PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    set (SRC_LIST ${SRC_LIST}
        src/ntKludges.C
        # src/dthread-win.C
        # src/dthread.C
        src/addrtranslate-win.C
    )
    list(APPEND _PRIVATE_COMPILE_DEFS WIN32 _WIN32_WINNT=0x500)
endif()

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${C_SRC_LIST} PROPERTIES LANGUAGE C)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" AND ${CMAKE_CXX_COMPILER_VERSION} MATCHES "^[789](\.|$)")
    # Disable var-tracking-assignments for arch-x86.C for gcc 7, 8, & 9.
    # The default max size for these compilers is too small so it fails,
    # adjusting it using
    #
    #   PROPERTIES COMPILE_FLAGS "--param=max-vartrack-size=600000000"
    #
    # succeeds, but just disable it.
    set_source_files_properties(src/arch-x86.C
        PROPERTIES COMPILE_FLAGS "-fno-var-tracking-assignments")
endif()

# adjust warning threshold if set in cmake/warnings.cmake
if (debugMaxFrameSizeOverrideSyscallInformation)
    set_source_files_properties(src/MachSyscall.C PROPERTIES COMPILE_FLAGS
		$<$<CONFIG:DEBUG>:-Wframe-larger-than=${debugMaxFrameSizeOverrideSyscallInformation}>)
endif()

if (nonDebugMaxFrameSizeOverrideSyscallInformation)
    set_source_files_properties(src/MachSyscall.C PROPERTIES COMPILE_FLAGS
		$<$<NOT:$<CONFIG:DEBUG>>:-Wframe-larger-than=${nonDebugMaxFrameSizeOverrideSyscallInformation}>)
endif()

if(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    set(_PRIVATE_LINK_LIBS Psapi WS2_32 dbghelp)
endif()

dyninst_library(common
    SOURCES
        ${SRC_LIST}
        ${C_SRC_LIST}
    COMPILE_DEFINITIONS
        PRIVATE COMMON_LIB ${_PRIVATE_COMPILE_DEFS}
    INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/h>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    DEPENDS
        LibIberty-External
        TBB-External
    LINK_LIBRARIES
        Dyninst::cap-arch-def
        Dyninst::Boost
        Dyninst::TBB
        PRIVATE
            Dyninst::LibIberty
            ${_PRIVATE_LINK_LIBS})
