# CMake configuration for proccontrol directory

include_directories(src h)

set(SRC_LIST
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
if(UNIX)
    set(SRC_LIST ${SRC_LIST} src/sysv.C src/int_thread_db.C src/mmapalloc.C)
elseif(WIN32)
    set(SRC_LIST
        ${SRC_LIST}
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
    set(SRC_LIST
        ${SRC_LIST}
        src/freebsd.C
        src/unix.C
        src/notify_pipe.C
        ../common/src/dthread-unix.C
        ../common/src/dthread.C
        src/loadLibrary/codegen-freebsd.C)
elseif(DYNINST_OS_Linux)
    set(SRC_LIST
        ${SRC_LIST}
        src/linux.C
        src/unix.C
        src/notify_pipe.C
        ../common/src/dthread-unix.C
        ../common/src/dthread.C
        src/loadLibrary/codegen-linux.C)
endif()

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)

add_definitions(-DPROCCONTROL_EXPORTS)

set(DEPS common ${SYMREADER})

dyninst_library(pcontrol ${DEPS})

target_link_private_libraries(pcontrol ${CMAKE_DL_LIBS})
target_link_libraries(pcontrol PUBLIC Dyninst::Boost_headers)
target_link_libraries(pcontrol PRIVATE Dyninst::Thread_DB)

if(UNIX)
    # Boost auto-links on Windows; don't double-link
    target_link_private_libraries(pcontrol pthread)
endif()
