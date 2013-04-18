# CMake configuration for proccontrol directory

include_directories (
    ${PROJECT_SOURCE_DIR}/proccontrol/src
  )

set (SRC_LIST
     src/event.C 
     src/generator.C 
     src/handler.C 
     src/mailbox.C 
     src/process.C 
     src/pcerrors.C 
     src/procpool.C 
     src/irpc.C 
     src/response.C 
     src/int_thread_db.C 
     src/memcache.C 
     src/sysv.C 
     src/x86_process.C 
     src/ppc_process.C 
     src/mmapalloc.C 
     src/procset.C 
     src/processplat.C 
     src/loadLibrary/injector.C 
     src/loadLibrary/codegen.C 
     src/loadLibrary/codegen-ppc.C 
     src/loadLibrary/codegen-x86.C
 )

if (PLATFORM MATCHES freebsd)
set (SRC_LIST ${SRC_LIST}
     src/freebsd.C
     src/unix.C
     src/notify_pipe.C
     ../common/src/dthread-unix.C
     src/loadLibrary/codegen-freebsd.C
  )
elseif (PLATFORM MATCHES linux)
set (SRC_LIST ${SRC_LIST}
     src/linux.C
     src/unix.C
     src/notify_pipe.C
     ../common/src/dthread-unix.C
     src/loadLibrary/codegen-linux.C
  )
elseif (PLATFORM MATCHES bgq)
set (SRC_LIST ${SRC_LIST}
     src/bluegeneq.C
     src/bgq-messages.C
     src/notify_pipe.C
     ../common/src/dthread-unix.C
     src/loadLibrary/codegen-linux.C
  )
elseif (PLATFORM MATCHES cnl)
set (SRC_LIST ${SRC_LIST}
     src/linux.C
     src/unix.C
     src/notify_pipe.C
     ../common/src/dthread-unix.C
     src/loadLibrary/codegen-stub.C
  )
elseif (PLATFORM MATCHES windows)
endif()

add_library (pcontrol ${SRC_LIST})
target_link_libraries(pcontrol common)

if (UNIX)
target_link_libraries(pcontrol dynElf)
target_link_libraries(pcontrol symLite)
endif()

FILE (GLOB headers "h/*.h")
set_target_properties (pcontrol PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS pcontrol
        EXPORT dyninst
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
