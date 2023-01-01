include_directories(${PROJECT_SOURCE_DIR}/dyninstAPI_RT/src ${PROJECT_SOURCE_DIR}/dyninstAPI_RT/h
                    ${PROJECT_SOURCE_DIR})

set(SRC_LIST src/RTcommon.c)

if(DYNINST_OS_FreeBSD)
    set(SRC_LIST
        ${SRC_LIST}
        src/RTposix.c
        src/RTfreebsd.c
        src/RTheap.c
        src/RTheap-freebsd.c
        src/RTthread.c
        src/RTspace.S
        src/RTsignal.c)
elseif(DYNINST_OS_Linux)
    set(SRC_LIST
        ${SRC_LIST}
        src/RTposix.c
        src/RTlinux.c
        src/RTheap.c
        src/RTheap-linux.c
        src/RTthread.c
        src/RTspace.S
        src/RTsignal.c)
    set(RT_STATIC_ONLY_SRC_LIST
        ${RT_STATIC_ONLY_SRC_LIST} src/RTstatic_ctors_dtors_begin.c
        src/RTstatic_ctors_dtors_end.c)
elseif(DYNINST_OS_Windows)
    set(SRC_LIST ${SRC_LIST} src/RTheap.c src/RTheap-win.c src/RTwinnt.c src/RTthread.c
                 src/RTthread-x86.c)
endif()

set(SRC_LIST_i386 src/RTthread-x86.c src/RTtlsgetaddr-x86.S)
set(RT_STATIC_ONLY_SRC_LIST_i386 src/RTstatic_ctors_dtors-x86.c)
set(SRC_LIST_x86_64 src/RTthread-x86-64.c src/RTtlsgetaddr-x86.S)
set(RT_STATIC_ONLY_SRC_LIST_x86_64 src/RTstatic_ctors_dtors-x86.c)
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
set_source_files_properties(${SRC_ASSEMBLY} PROPERTIES LANGUAGE C)

# The arch-specific files other than RTthread-x86 are Unix-only.
if(DYNINST_OS_UNIX)
    if(DYNINST_ARCH_x86_64)
        set(SRC_LIST_mabi ${SRC_LIST} ${SRC_LIST_i386})
        set(RT_STATIC_ONLY_SRC_LIST_mabi ${RT_STATIC_ONLY_SRC_LIST}
                                         ${RT_STATIC_ONLY_SRC_LIST_i386})
        set(SRC_LIST ${SRC_LIST} ${SRC_LIST_x86_64})
        set(RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST}
                                    ${RT_STATIC_ONLY_SRC_LIST_x86_64})
    elseif(DYNINST_ARCH_ppc64le)
        set(SRC_LIST_mabi ${SRC_LIST})
        set(RT_STATIC_ONLY_SRC_LIST_mabi ${RT_STATIC_ONLY_SRC_LIST})
        set(SRC_LIST ${SRC_LIST} ${SRC_LIST_ppc64})
        set(RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST}
                                    ${RT_STATIC_ONLY_SRC_LIST_ppc64})
    elseif(DYNINST_ARCH_i386)
        set(SRC_LIST ${SRC_LIST} ${SRC_LIST_i386})
        set(RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST}
                                    ${RT_STATIC_ONLY_SRC_LIST_i386})
    elseif(DYNINST_ARCH_aarch64)
        set(SRC_LIST ${SRC_LIST} ${SRC_LIST_aarch64})
        set(RT_STATIC_ONLY_SRC_LIST ${RT_STATIC_ONLY_SRC_LIST}
                                    ${RT_STATIC_ONLY_SRC_LIST_aarch64})
    endif()
endif()

add_library(dyninstAPI_RT SHARED ${SRC_LIST})

if(NOT DYNINST_OS_Windows)
    target_link_libraries(dyninstAPI_RT PRIVATE ${CMAKE_DL_LIBS})
else() # windows
    target_link_libraries(dyninstAPI_RT PRIVATE ws2_32 dbghelp psapi)
endif()

add_library(dyninstAPI_RT_static STATIC ${SRC_LIST} ${RT_STATIC_ONLY_SRC_LIST})

# this should carry over from CMakeLists.txt
set_target_properties(
    dyninstAPI_RT
    PROPERTIES OUTPUT_NAME dyninstAPI_RT
               SOVERSION ${DYNINST_SOVERSION}
               VERSION ${DYNINST_LIBVERSION})

# this will not
set_target_properties(
    dyninstAPI_RT_static PROPERTIES OUTPUT_NAME dyninstAPI_RT COMPILE_DEFINITIONS
                                                              DYNINST_RT_STATIC_LIB)

set_target_properties(dyninstAPI_RT PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(dyninstAPI_RT_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

message(STATUS "dyninstAPI RT library SOVERSION: ${DYNINST_SOVERSION}")
message(STATUS "dyninstAPI RT library LIBVERSION: ${DYNINST_LIBVERSION}")

file(GLOB headers "h/*.h")
# append, you fool!
set_target_properties(dyninstAPI_RT dyninstAPI_RT_static PROPERTIES PUBLIC_HEADER
                                                                    "${headers}")

install(
    TARGETS dyninstAPI_RT dyninstAPI_RT_static
    EXPORT DyninstTargets
    RUNTIME DESTINATION ${INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

# Test to see if we want the 32-bit library

if(SRC_LIST_mabi)

    # The following code is from CheckCCompilerFlag.cmake. It uses a simple program that
    # can be compiled even if 32-bit includes are missing; we need to extend it somewhat

    include(CheckCSourceCompiles)

    macro(CHECK_C_COMPILER_FLAG_EXTENDED _FLAG _RESULT)
        set(SAFE_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        set(CMAKE_C_FLAGS "${_FLAG}")

        # Addition here: use signal.h
        check_c_source_compiles(
            "#include <signal.h>
   int main(void) { return 0; }"
            ${_RESULT}
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
        set(CMAKE_C_FLAGS "${SAFE_CMAKE_C_FLAGS}")
    endmacro(CHECK_C_COMPILER_FLAG_EXTENDED)

    message(STATUS "Checking for 32-bit runtime library...")

    check_c_compiler_flag_extended("-m32" CHECK_RT_LIB_32)
    if(CHECK_RT_LIB_32 AND NOT ${BUILD_RTLIB_32} MATCHES "OFF")
        message(
            STATUS
                "Enabling 32-bit runtime library; change BUILD_RTLIB_32 to OFF to disable"
            )
        set(BUILD_RTLIB_32 ON)
    else()
        message(
            STATUS
                "Disabling 32-bit runtime library; change BUILD_RTLIB_32 to ON and install 32-bit build environment to enable"
            )
        set(BUILD_RTLIB_32 OFF)
    endif()

    if(BUILD_RTLIB_32 MATCHES "ON")

        add_library(dyninstAPI_RT_m32 SHARED ${SRC_LIST_mabi})
        target_link_libraries(dyninstAPI_RT_m32 ${CMAKE_DL_LIBS})
        add_library(dyninstAPI_RT_m32_static STATIC ${SRC_LIST_mabi}
                                                    ${RT_STATIC_ONLY_SRC_LIST_mabi})

        set_target_properties(
            dyninstAPI_RT_m32
            PROPERTIES COMPILE_FLAGS "-DMUTATEE_32 -m32"
                       LINK_FLAGS "-m32"
                       OUTPUT_NAME dyninstAPI_RT_m32
                       SOVERSION ${DYNINST_SOVERSION}
                       VERSION ${DYNINST_LIBVERSION})

        set_target_properties(
            dyninstAPI_RT_m32_static
            PROPERTIES OUTPUT_NAME dyninstAPI_RT_m32
                       COMPILE_FLAGS "-DMUTATEE_32 -m32"
                       LINK_FLAGS "-m32"
                       COMPILE_DEFINITIONS DYNINST_RT_STATIC_LIB)

        set_target_properties(dyninstAPI_RT_m32 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
        set_target_properties(dyninstAPI_RT_m32_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

        install(
            TARGETS dyninstAPI_RT_m32 dyninstAPI_RT_m32_static
            EXPORT DyninstTargets
            RUNTIME DESTINATION ${INSTALL_LIB_DIR}
            LIBRARY DESTINATION ${INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
            PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

        install(EXPORT DyninstTargets DESTINATION "${INSTALL_CMAKE_DIR}")

    endif()
endif()
