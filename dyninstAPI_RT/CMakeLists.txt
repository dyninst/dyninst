# CMake configuration for dyninstAPI_RT directory

include_directories (
    ${PROJECT_SOURCE_DIR}/dyninstAPI_RT/src
  )

set (SRC_LIST
    src/RTcommon.c 
    src/RTmemEmulator.c
    src/RTsignal.c 
)

if (PLATFORM MATCHES freebsd)
set (SRC_LIST ${SRC_LIST}
    src/RTposix.c
    src/RTfreebsd.c 
    src/RTheap.c 
    src/RTheap-freebsd.c 
    src/RTthread.c 
    src/RTspace.s 
)
elseif (PLATFORM MATCHES linux OR PLATFORM MATCHES bgq)
set (SRC_LIST ${SRC_LIST}
    src/RTposix.c 
    src/RTlinux.c 
    src/RTheap.c 
    src/RTheap-linux.c 
    src/RTthread.c 
    src/RTspace.s 
)
endif ()

set (SRC_LIST_i386
    src/RTthread-x86.c 
    src/RTtlsgetaddr-x86.S 
    src/RTstatic_ctors_dtors-x86.c
)
set (SRC_LIST_x86_64
    src/RTthread-x86-64.c 
    src/RTtlsgetaddr-x86.S 
    src/RTstatic_ctors_dtors-x86.c
)
set (SRC_LIST_ppc32
    src/RTthread-powerpc.c
    src/RTthread-powerpc-asm.S
    src/RTstatic_ctors_dtors-ppc32.c
)
set (SRC_LIST_ppc64
    src/RTthread-powerpc.c
    src/RTthread-powerpc-asm.S
    src/RTstatic_ctors_dtors-ppc64.c                                                                                         
)

if (PLATFORM MATCHES amd64 OR PLATFORM MATCHES x86_64)
set (SRC_LIST_mabi ${SRC_LIST} ${SRC_LIST_i386})
set (SRC_LIST ${SRC_LIST} ${SRC_LIST_x86_64})
elseif (PLATFORM MATCHES ppc64)
set (SRC_LIST_mabi ${SRC_LIST} ${SRC_LIST_ppc32})
set (SRC_LIST ${SRC_LIST} ${SRC_LIST_ppc64}) 
elseif (PLATFORM MATCHES i386)
set (SRC_LIST ${SRC_LIST} ${SRC_LIST_i386})
elseif (PLATFORM MATCHES ppc32)
set (SRC_LIST ${SRC_LIST} ${SRC_LIST_ppc32})
endif()


add_library (dyninstAPI_RT SHARED ${SRC_LIST})
add_library (dyninstAPI_RT_static STATIC ${SRC_LIST})

set_target_properties (dyninstAPI_RT
                      PROPERTIES
                      SOVERSION ${SOVERSION}
                      VERSION ${LIBVERSION})

set_target_properties (dyninstAPI_RT_static
                      PROPERTIES
                      OUTPUT_NAME dyninstAPI_RT
                      SOVERSION ${SOVERSION}
                      VERSION ${LIBVERSION})

INSTALL (DIRECTORY h/ DESTINATION include FILES_MATCHING PATTERN "*.h")
INSTALL (TARGETS dyninstAPI_RT dyninstAPI_RT_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# Test to see if we want the 32-bit library

if (SRC_LIST_mabi) 

# The following code is from CheckCCompilerFlag.cmake. It uses a
# simple program that can be compiled even if 32-bit includes are 
# missing; we need to extend it somewhat

INCLUDE(CheckCSourceCompiles)

MACRO (CHECK_C_COMPILER_FLAG_EXTENDED _FLAG _RESULT)
   SET(SAFE_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
   SET(CMAKE_C_FLAGS "${_FLAG}")

# Addition here: use signal.h
   CHECK_C_SOURCE_COMPILES("#include <signal.h>
   int main(void) { return 0; }" 
        ${_RESULT}
# End addition
     # Some compilers do not fail with a bad flag
     FAIL_REGEX "warning: command line option .* is valid for .* but not for C"
                                                            # Apple gcc
     FAIL_REGEX "unrecognized .*option"                     # GNU
     FAIL_REGEX "unknown .*option"                          # Clang
     FAIL_REGEX "ignoring unknown option"                   # MSVC
     FAIL_REGEX "warning D9002"                             # MSVC, any lang
     FAIL_REGEX "[Uu]nknown option"                         # HP
     FAIL_REGEX "[Ww]arning: [Oo]ption"                     # SunPro
     FAIL_REGEX "command option .* is not recognized"       # XL
     )
   SET(CMAKE_C_FLAGS "${SAFE_CMAKE_C_FLAGS}")
ENDMACRO (CHECK_C_COMPILER_FLAG_EXTENDED)


message (STATUS "Checking for 32-bit runtime library...")

CHECK_C_COMPILER_FLAG_EXTENDED ("-m32" CHECK_RT_LIB_32)
if (CHECK_RT_LIB_32)
message (STATUS "Enabling 32-bit runtime library; change BUILD_RTLIB_32 to OFF to disable")
SET (BUILD_RTLIB_32 ON CACHE STRING "Build 32-bit version of RT library")
ELSE ()
SET (BUILD_RTLIB_32 OFF CACHE STRING "Build 32-bit version of RT library")
ENDIF()


if (BUILD_RTLIB_32 MATCHES "ON")
set_property(SOURCE src/RTspace.s APPEND PROPERTY COMPILE_FLAGS "--32")

add_library (dyninstAPI_RT_m32 SHARED ${SRC_LIST_mabi})
add_library (dyninstAPI_RT_m32_static STATIC ${SRC_LIST_mabi})

set_target_properties (dyninstAPI_RT_m32
                      PROPERTIES
                      COMPILE_FLAGS "-m32"
                      LINK_FLAGS "-m32"
                      SOVERSION ${SOVERSION}
                      VERSION ${LIBVERSION})

set_target_properties (dyninstAPI_RT_m32_static
                      PROPERTIES
                      OUTPUT_NAME dyninstAPI_RT_m32
                      COMPILE_FLAGS "-m32"
                      LINK_FLAGS "-m32"
                      SOVERSION ${SOVERSION}
                      VERSION ${LIBVERSION})

INSTALL (TARGETS dyninstAPI_RT_m32 dyninstAPI_RT_m32_static
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)


endif()
endif()
