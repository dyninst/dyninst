include_guard(GLOBAL)

include(DyninstLibrary)

set(_public_headers
    h/Decoder.h
    h/Event.h
    h/EventType.h
    h/Handler.h
    h/Mailbox.h
    h/PCErrors.h
    h/PCProcess.h
    h/PlatFeatures.h
    h/ProcessSet.h)

set(_private_headers
    src/arm_process.h
    src/DecoderWindows.h
    src/freebsd.h
    src/GeneratorWindows.h
    src/int_event.h
    src/int_handler.h
    src/int_process.h
    src/int_thread_db.h
    src/irpc.h
    src/linux.h
    src/loadLibrary/codegen.h
    src/loadLibrary/injector.h
    src/memcache.h
    src/mmapalloc.h
    src/notify_pipe.h
    src/ppc_process.h
    src/processplat.h
    src/procpool.h
    src/proc_service_wrapper.h
    src/resp.h
    src/response.h
    src/snippets.h
    src/sysv.h
    src/unix.h
    src/windows_handler.h
    src/windows_process.h
    src/windows_thread.h
    src/x86_process.h)

set(_sources
    src/event.C
    src/generator.C
    src/handler.C
    src/mailbox.C
    src/process.C
    src/pcerrors.C
    src/procpool.C
    src/irpc.C
    src/response.C
    src/resp.C
    src/memcache.C
    src/procset.C
    src/processplat.C
    src/loadLibrary/injector.C
    src/loadLibrary/codegen.C
    src/loadLibrary/codegen-x86.C
    src/loadLibrary/codegen-ppc.C
    src/loadLibrary/codegen-aarch64.C
    src/x86_process.C
    src/ppc_process.C
    src/arm_process.C
    src/pcmachsyscall.C)

if(DYNINST_OS_UNIX)
  list(APPEND _sources src/sysv.C src/int_thread_db.C src/mmapalloc.C)
elseif(DYNINST_OS_Windows)
  list(
    APPEND
    _sources
    src/GeneratorWindows.C
    src/DecoderWindows.C
    src/windows_handler.C
    src/windows_process.C
    src/windows_thread.C
    src/loadLibrary/codegen-win.C
    ../common/src/dthread-win.C
    ../common/src/dthread.C)
endif()

if(DYNINST_OS_FreeBSD)
  list(
    APPEND
    _sources
    src/freebsd.C
    src/unix.C
    src/notify_pipe.C
    ../common/src/dthread-unix.C
    ../common/src/dthread.C
    src/loadLibrary/codegen-freebsd.C)
elseif(DYNINST_OS_Linux)
  list(
    APPEND
    _sources
    src/linux.C
    src/unix.C
    src/notify_pipe.C
    ../common/src/dthread-unix.C
    ../common/src/dthread.C
    src/loadLibrary/codegen-linux.C)
endif()

# cmake-format: off
dyninst_library(
  pcontrol
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DEFINES PROCCONTROL_EXPORTS
  DYNINST_DEPS common ${SYMREADER}
  PUBLIC_DEPS Dyninst::Boost_headers
  PRIVATE_DEPS ${CMAKE_DL_LIBS} Dyninst::Thread_DB Threads::Threads
)
# cmake-format: on

# When using symlite, we still need the symtabAPI headers
if(LIGHTWEIGHT_SYMTAB)
  foreach(t ${pcontrol_TARGETS})
    target_include_directories(
      ${t} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/symtabAPI/h>")
  endforeach()
endif()
