# C extensions are required
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------#
# target sources
# ------------------------------------------------------------------------------#

set(SRC_LIST src/RTcommon.c src/RTmemEmulator.c)

if(PLATFORM MATCHES freebsd)
    list(
        APPEND
        SRC_LIST
        src/RTposix.c
        src/RTfreebsd.c
        src/RTheap.c
        src/RTheap-freebsd.c
        src/RTthread.c
        src/RTspace.S
        src/RTsignal.c)
elseif(PLATFORM MATCHES linux)
    list(
        APPEND
        SRC_LIST
        src/RTposix.c
        src/RTlinux.c
        src/RTheap.c
        src/RTheap-linux.c
        src/RTthread.c
        src/RTspace.S
        src/RTsignal.c)
    list(APPEND RT_STATIC_ONLY_SRC_LIST src/RTstatic_ctors_dtors_begin.c
         src/RTstatic_ctors_dtors_end.c)
elseif(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    list(
        APPEND
        SRC_LIST
        src/RTheap.c
        src/RTheap-win.c
        src/RTwinnt.c
        src/RTthread.c
        src/RTthread-x86.c)
endif()

set(SRC_LIST_i386 src/RTthread-x86.c src/RTtlsgetaddr-x86.S)
set(RT_STATIC_ONLY_SRC_LIST_i386 src/RTstatic_ctors_dtors-x86.c)
set(SRC_LIST_x86_64 src/RTthread-x86-64.c src/RTtlsgetaddr-x86.S)
set(RT_STATIC_ONLY_SRC_LIST_x86_64 src/RTstatic_ctors_dtors-x86.c)
set(SRC_LIST_ppc32 src/RTthread-powerpc.c src/RTthread-powerpc-asm.S)
set(RT_STATIC_ONLY_SRC_LIST_ppc32 src/RTstatic_ctors_dtors-ppc32.c)
set(SRC_LIST_ppc64 src/RTthread-powerpc.c src/RTthread-powerpc-asm.S)
set(RT_STATIC_ONLY_SRC_LIST_ppc64 src/RTstatic_ctors_dtors-ppc64.c)
set(SRC_LIST_aarch64 src/RTthread-aarch64.c
                     # src/RTthread-aarch64-asm.S
    )
set(RT_STATIC_ONLY_SRC_LIST_aarch64 src/RTstatic_ctors_dtors-aarch64.c)

# We use gcc to compile the various assembly files, but cmake doesn't default to knowing
# that gcc can handle .S.
enable_language(ASM)
file(GLOB SRC_ASSEMBLY "src/*.S")
if(NEED_NATIVE_ASSEMBER)
    set_source_files_properties(${SRC_ASSEMBLY} PROPERTIES LANGUAGE ASM)
else()
    set_source_files_properties(${SRC_ASSEMBLY} PROPERTIES LANGUAGE C)
endif()

# The arch-specific files other than RTthread-x86 are Unix-only.
if(UNIX)
    if(PLATFORM MATCHES amd64 OR PLATFORM MATCHES x86_64)
        set(SRC_LIST_mabi ${SRC_LIST} ${SRC_LIST_i386})
        set(RT_STATIC_ONLY_SRC_LIST_mabi ${RT_STATIC_ONLY_SRC_LIST}
                                         ${RT_STATIC_ONLY_SRC_LIST_i386})
        list(APPEND SRC_LIST ${SRC_LIST_x86_64})
        list(APPEND RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST_x86_64})
    elseif(PLATFORM MATCHES ppc64)
        set(SRC_LIST_mabi ${SRC_LIST} ${SRC_LIST_ppc32})
        set(RT_STATIC_ONLY_SRC_LIST_mabi ${RT_STATIC_ONLY_SRC_LIST}
                                         ${RT_STATIC_ONLY_SRC_LIST_ppc32})
        list(APPEND SRC_LIST ${SRC_LIST_ppc64})
        list(APPEND RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST_ppc64})
    elseif(PLATFORM MATCHES i386)
        list(APPEND SRC_LIST ${SRC_LIST_i386})
        list(APPEND RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST_i386})
    elseif(PLATFORM MATCHES ppc32)
        list(APPEND SRC_LIST ${SRC_LIST_ppc32})
        list(APPEND RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST_ppc32})
    elseif(PLATFORM MATCHES aarch64)
        list(APPEND SRC_LIST ${SRC_LIST_aarch64})
        list(APPEND RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST_aarch64})
    endif()
endif()

dyninst_library(
    dyninstAPI_RT BUILD_STATIC BUILD_SHARED
    SOURCES ${SRC_LIST}
    INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/h>
    LINK_LIBRARIES
        Dyninst::headers
        Dyninst::Boost
        Dyninst::TBB
        PRIVATE
        Dyninst::dynDL
        Dyninst::dynWinWs2_32
        Dyninst::dynWinDbgHelp
        Dyninst::dynWinPsapi)

if(TARGET dyninstAPI_RT-static)
    target_sources(dyninstAPI_RT-static PRIVATE ${RT_STATIC_ONLY_SRC_LIST})
    target_compile_definitions(dyninstAPI_RT-static PRIVATE DYNINST_RT_STATIC_LIB)
endif()

# Test to see if we want the 32-bit library

if(SRC_LIST_mabi)
    # The following code is from CheckCCompilerFlag.cmake. It uses a simple program that
    # can be compiled even if 32-bit includes are missing; we need to extend it somewhat

    include(CheckCSourceCompiles)

    function(CHECK_C_COMPILER_FLAG_EXTENDED _FLAG _RESULT)
        set(CMAKE_C_FLAGS "${_FLAG}")

        # Addition here: use signal.h
        dyninst_begin_flag_check()
        check_c_source_compiles(
            "#include <signal.h>
            int main(void) { return 0; }"
            _COMPILE_RESULT
            # End addition Some compilers do not fail with a bad flag
            FAIL_REGEX
            "warning: command line option .* is valid for .* but not for C"
            # Apple gcc
            FAIL_REGEX
            "unrecognized .*option" # GNU
            FAIL_REGEX
            "unknown .*option" # Clang
            FAIL_REGEX
            "ignoring unknown option" # MSVC
            FAIL_REGEX
            "warning D9002" # MSVC, any lang
            FAIL_REGEX
            "[Uu]nknown option" # HP
            FAIL_REGEX
            "[Ww]arning: [Oo]ption" # SunPro
            FAIL_REGEX
            "command option .* is not recognized" # XL
            )
        dyninst_end_flag_check()
        set(${_RESULT}
            ${_COMPILE_RESULTS}
            PARENT_SCOPE)
    endfunction()

    dyninst_message(STATUS "Checking for 32-bit runtime library...")

    check_c_compiler_flag_extended("-m32" CHECK_RT_LIB_32)
    if(CHECK_RT_LIB_32 AND NOT ${BUILD_RTLIB_32} MATCHES "OFF")
        dyninst_message(
            STATUS
            "Enabling 32-bit runtime library; change BUILD_RTLIB_32 to OFF to disable")
        set(BUILD_RTLIB_32 ON)
    else()
        dyninst_message(
            STATUS
            "Disabling 32-bit runtime library; change BUILD_RTLIB_32 to ON and install 32-bit build environment to enable"
            )
        set(BUILD_RTLIB_32 OFF)
    endif()

    if(BUILD_RTLIB_32)
        set(USE_OpenMP OFF)
        dyninst_library(
            dyninstAPI_RT_m32 BUILD_STATIC BUILD_SHARED SKIP_OPENMP
            SOURCES ${SRC_LIST_mabi}
            COMPILE_DEFINITIONS MUTATEE_32
            COMPILE_OPTIONS -m32
            LINK_OPTIONS -m32
            LINK_LIBRARIES Dyninst::headers
            INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/h>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

        if(TARGET dyninstAPI_RT_m32-static)
            target_sources(dyninstAPI_RT_m32-static
                           PRIVATE ${RT_STATIC_ONLY_SRC_LIST_mabi})
            target_compile_definitions(dyninstAPI_RT_m32-static
                                       PRIVATE DYNINST_RT_STATIC_LIB)
        endif()
    endif()
endif()
