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
    src/RTspace.S 
)
elseif (PLATFORM MATCHES linux OR PLATFORM MATCHES bgq)
set (SRC_LIST ${SRC_LIST}
    src/RTposix.c 
    src/RTlinux.c 
    src/RTheap.c 
    src/RTheap-linux.c 
    src/RTthread.c 
    src/RTspace.S 
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

include (CheckCCompilerFlag)
message ("Checking for 32-bit runtime library...")

# For some reason this test borks with HAS_M32
CHECK_C_COMPILER_FLAG ("-O2" BUILD_MABI_RTLIB)

if (BUILD_MABI_RTLIB)
SET (BUILD_RTLIB_32 ON CACHE STRING "Build 32-bit version of RT library")

if (BUILD_RTLIB_32)
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
endif()

