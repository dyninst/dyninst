include_guard(GLOBAL)

include(DyninstLibrary)

set(_public_headers h/dyninstAPI_RT.h h/dyninstRTExport.h)

set(_private_headers src/RTcommon.h src/RTheap.h src/RTthread.h)

set(_sources src/RTcommon.c src/RTheap.c src/RTthread.c)

if(DYNINST_OS_FreeBSD)
  list(
    APPEND
    _sources
    src/RTposix.c
    src/RTfreebsd.c
    src/RTheap-freebsd.c
    src/RTspace.S
    src/RTsignal.c)
elseif(DYNINST_OS_Linux)
  set(_static_sources src/RTstatic_ctors_dtors_begin.c src/RTstatic_ctors_dtors_end.c)
  list(
    APPEND
    _sources
    src/RTposix.c
    src/RTlinux.c
    src/RTheap-linux.c
    src/RTspace.S
    src/RTsignal.c)
elseif(DYNINST_OS_Windows)
  list(APPEND _sources src/RTheap-win.c src/RTwinnt.c src/RTthread-x86.c)
endif()

# The arch-specific files other than RTthread-x86 are Unix-only.
if(DYNINST_OS_UNIX)
  if(DYNINST_HOST_ARCH_X86_64)
    list(APPEND _sources src/RTthread-x86-64.c src/RTtlsgetaddr-x86.S)
  elseif(DYNINST_HOST_ARCH_I386)
    list(APPEND _sources src/RTthread-x86.c src/RTtlsgetaddr-x86.S)
  elseif(DYNINST_HOST_ARCH_PPC64LE)
    list(APPEND _sources src/RTthread-powerpc.c src/RTthread-powerpc-asm.S)
  elseif(DYNINST_HOST_ARCH_AARCH64)
    list(APPEND _sources src/RTthread-aarch64.c)
  endif()

  if(DYNINST_HOST_ARCH_X86_64)
    list(APPEND _static_sources src/RTstatic_ctors_dtors-x86.c)
  elseif(DYNINST_HOST_ARCH_I386)
    list(APPEND _static_sources src/RTstatic_ctors_dtors-x86.c)
  elseif(DYNINST_HOST_ARCH_PPC64LE)
    list(APPEND _static_sources src/RTstatic_ctors_dtors-ppc64.c)
  elseif(DYNINST_HOST_ARCH_AARCH64)
    list(APPEND _static_sources src/RTstatic_ctors_dtors-aarch64.c)
  endif()
endif()

# cmake-format: off
dyninst_library(
  dyninstAPI_RT
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DYNINST_DEPS common
  PRIVATE_DEPS Threads::Threads
  FORCE_STATIC
)
# cmake-format: on

foreach(t ${dyninstAPI_RT_TARGETS})
  if(DYNINST_OS_UNIX)
    target_link_libraries(${t} PRIVATE ${CMAKE_DL_LIBS})
  elseif(DYNINST_OS_Windows)
    target_link_libraries(${t} PRIVATE ws2_32 dbghelp psapi)
  endif()

  if(${t} MATCHES "static")
    target_sources(${t} PRIVATE ${_static_sources})
    target_compile_definitions(${t} PRIVATE DYNINST_RT_STATIC_LIB)
  endif()

  # Both shared and static should be named 'dyninstAPI_RT'- only extension differs
  set_property(TARGET ${t} PROPERTY OUTPUT_NAME dyninstAPI_RT)
endforeach()

if(BUILD_RTLIB_32)
  include(DyninstCheck32bitRuntimeSupported)
  dyninst_check_32bit_runtime_supported(LANGUAGES C)

  if(NOT DYNINST_C_32BIT_RUNTIME_SUPPORTED)
    message(
      FATAL_ERROR "BUILD_RTLIB_32 enabled, but compiler doesn't support 32-bit runtime")
  endif()

  if(ENABLE_STATIC_LIBS)
    if(NOT DYNINST_C_32BIT_STATIC_RUNTIME_SUPPORTED)
      message(FATAL_ERROR "32-bit static runtime not supported")
    endif()
    set(_force_static FORCE_STATIC)
  endif()

  if(NOT DYNINST_OS_UNIX)
    message(FATAL_ERROR "32-bit runtime is only supported on Unixes")
  endif()

  if(NOT DYNINST_HOST_ARCH_X86_64 AND NOT DYNINST_HOST_ARCH_I386)
    message(FATAL_ERROR "32-bit runtime is only supported on i386 and x86_64")
  endif()

  if(DYNINST_HOST_ARCH_X86_64)
    # We're cross-compiling, so remove the x86_64 thread stuff
    list(FILTER _sources EXCLUDE REGEX "RTthread-x86-64")
  endif()

  # cmake-format: off
  # The headers are all listed as PRIVATE so that they do not get copied into the install
  # tree twice (they already get copied in by the creation of the shared library targets)
	dyninst_library(
	  dyninstAPI_RT_m32
	  PRIVATE_HEADER_FILES ${_public_headers} ${_private_headers}
	  SOURCE_FILES ${_sources} src/RTthread-x86.c src/RTtlsgetaddr-x86.S
	  DEFINES "MUTATEE_32"
	  PRIVATE_DEPS Threads::Threads
	  ${_force_static}
	)
	# cmake-format: on

  foreach(t ${dyninstAPI_RT_m32_TARGETS})
    target_compile_options(${t} PRIVATE "${DYNINST_C_32BIT_RUNTIME_FLAG}")
    target_link_options(${t} PRIVATE "${DYNINST_C_32BIT_RUNTIME_FLAG}")

    target_include_directories(
      ${t} BEFORE PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common/h>")

    if(DYNINST_OS_UNIX)
      target_link_libraries(${t} PRIVATE ${CMAKE_DL_LIBS})
    elseif(DYNINST_OS_Windows)
      target_link_libraries(${t} PRIVATE ws2_32 dbghelp psapi)
    endif()

    if(${t} MATCHES "static")
      # Both shared and static should be named 'dyninstAPI_RT_m32'- only extension differs
      set_property(TARGET ${t} PROPERTY OUTPUT_NAME dyninstAPI_RT_m32)

      target_sources(${t} PRIVATE ${static_sources} src/RTstatic_ctors_dtors-x86.c)
      target_compile_definitions(${t} PRIVATE DYNINST_RT_STATIC_LIB)
      target_compile_options(${t} PRIVATE "${DYNINST_C_32BIT_STATIC_RUNTIME_FLAG}")
      target_link_options(${t} PRIVATE "${DYNINST_C_32BIT_STATIC_RUNTIME_FLAG}")
    endif()
  endforeach()
endif()
