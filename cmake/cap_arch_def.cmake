#
# -- Define the capabilities for each supported architecture/platform
#
#  cap_32_64 - This host 64-bit platform supports modifying 32-bit binaries
#

set (CAP_DEFINES
     -Dcap_dynamic_heap 
     -Dcap_liveness 
     -Dcap_threads
)

if (PLATFORM MATCHES i386)
set (ARCH_DEFINES -Darch_x86)
set (CAP_DEFINES ${CAP_DEFINES}
             -Dcap_fixpoint_gen 
             -Dcap_noaddr_gen 
             -Dcap_stripped_binaries 
             -Dcap_tramp_liveness 
             -Dcap_virtual_registers
             -Dcap_stack_mods
    )

elseif (PLATFORM MATCHES x86_64 OR PLATFORM MATCHES amd64)
set (ARCH_DEFINES -Darch_x86_64 -Darch_64bit)
set (CAP_DEFINES ${CAP_DEFINES} 
             -Dcap_32_64
             -Dcap_fixpoint_gen 
             -Dcap_noaddr_gen
             -Dcap_registers
             -Dcap_stripped_binaries 
             -Dcap_tramp_liveness
             -Dcap_stack_mods
    )

elseif (PLATFORM MATCHES ppc64)
   set (ARCH_DEFINES -Darch_power -Darch_64bit)
   set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
   set (CAP_DEFINES ${CAP_DEFINES} 
                -Dcap_32_64
                -Dcap_registers
                -Dcap_toc_64
       )
elseif (PLATFORM MATCHES aarch64)
  #set (ARCH_DEFINES -Daarch_64 -Darch_64bit)
  set (ARCH_DEFINES -Darch_aarch64 -Darch_64bit)
  set (CAP_DEFINES ${CAP_DEFINES} -Dcap_registers)
endif (PLATFORM MATCHES i386)

if (PLATFORM MATCHES linux)
set (OS_DEFINES -Dos_linux)
set (CAP_DEFINES ${CAP_DEFINES} 
             -Dcap_async_events
             -Dcap_binary_rewriter
             -Dcap_dwarf
             -Dcap_mutatee_traps
             -Dcap_ptrace
    )
set (BUG_DEFINES -Dbug_syscall_changepc_rewind -Dbug_force_terminate_failure)

elseif (PLATFORM MATCHES freebsd)
set (OS_DEFINES -Dos_freebsd)
set (CAP_DEFINES ${CAP_DEFINES} 
             -Dcap_binary_rewriter
             -Dcap_dwarf
             -Dcap_mutatee_traps
    )
set (BUG_DEFINES -Dbug_freebsd_missing_sigstop 
             -Dbug_freebsd_mt_suspend 
             -Dbug_freebsd_change_pc 
             -Dbug_phdrs_first_page 
             -Dbug_syscall_changepc_rewind
    )

elseif (PLATFORM STREQUAL i386-unknown-nt4.0)
set (OS_DEFINES -Dos_windows)
set (CAP_DEFINES ${CAP_DEFINES} 
             -Dcap_mutatee_traps
    )
endif (PLATFORM MATCHES linux)


if (PLATFORM STREQUAL i386-unknown-linux2.4)
set (OLD_DEFINES -Di386_unknown_linux2_0)

elseif (PLATFORM STREQUAL x86_64-unknown-linux2.4)
set (OLD_DEFINES -Dx86_64_unknown_linux2_4)

elseif (PLATFORM STREQUAL ppc64_linux)
set (OLD_DEFINES -Dppc64_linux)
set (BUG_DEFINES ${BUG_DEFINES} -Dbug_registers_after_exit)

elseif (PLATFORM STREQUAL i386-unknown-freebsd7.2)
set (OLD_DEFINES -Di386_unknown_freebsd7_0)

elseif (PLATFORM STREQUAL amd64-unknown-freebsd7.2)
set (OLD_DEFINES -Damd64_unknown_freebsd7_0)

elseif (PLATFORM STREQUAL i386-unknown-nt4.0)
set (OLD_DEFINES -Di386_unknown_nt4_0)
elseif (PLATFORM STREQUAL aarch64-unknown-linux)
  set (OLD_DEFINES -Daarch64_unknown_linux)
else (PLATFORM STREQUAL i386-unknown-linux2.4)
  message (FATAL_ERROR "Unknown platform: ${PLATFORM}")
endif (PLATFORM STREQUAL i386-unknown-linux2.4)

if (THREAD_DB_FOUND)
message (STATUS "-- Enabling ThreadDB support")
set (CAP_DEFINES ${CAP_DEFINES} -Dcap_thread_db)
endif (THREAD_DB_FOUND)

set (UNIFIED_DEFINES ${CAP_DEFINES} ${BUG_DEFINES} ${ARCH_DEFINES} ${OS_DEFINES} ${OLD_DEFINES})

foreach (def ${UNIFIED_DEFINES})
  add_definitions (${def})
endforeach()


set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${UNIFIED_DEF_STRING}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${UNIFIED_DEF_STRING}")

message(STATUS "Set arch and platform based definitions")

