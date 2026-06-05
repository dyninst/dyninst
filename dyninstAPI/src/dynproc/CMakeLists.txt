include_guard(GLOBAL)

set(_headers
    dynProcess.h
    dynThread.h
    frameChecker.h
    frame.h
    infHeap.h
    nt_signal_emul.h
    pcEventHandler.h
    pcEventMuxer.h
    syscallNotification.h
    trapMappings.h)

set(_sources
    dynProcess.C
    dynThread.C
    frame.C
    frameChecker.C
    infHeap.C
    pcEventHandler.C
    pcEventMuxer.C)

if(DYNINST_OS_UNIX)
  list(APPEND _sources syscallNotification.C)
endif()

if(DYNINST_OS_FreeBSD)
  list(APPEND _sources syscall-freebsd.C)
elseif(DYNINST_OS_Linux)
  list(APPEND _sources syscall-linux.C)
elseif(DYNINST_OS_Windows)
  list(APPEND _sources syscall-nt.C)
endif()

if(DYNINST_HOST_ARCH_I386 OR DYNINST_HOST_ARCH_X86_64)
  list(APPEND _sources dynProcess-x86.C stackwalk-x86.C)
elseif(DYNINST_HOST_ARCH_PPC64LE)
  list(APPEND _sources dynProcess-power.C stackwalk-ppc.C)
elseif(DYNINST_HOST_ARCH_AARCH64)
  list(APPEND _sources dynProcess-aarch64.C stackwalk-aarch64.C)
endif()

# cmake-format: off
dyninst_library(
  dyninstAPI_dynproc
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common parseAPI patchAPI pcontrol
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)
# cmake-format: on

target_include_directories(dyninstAPI_dynproc PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
