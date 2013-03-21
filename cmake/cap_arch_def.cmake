
add_definitions (
             -Dcap_dynamic_heap 
             -Dcap_liveness 
             -Dcap_threads
    )

if (PLATFORM MATCHES i386)
add_definitions (-Darch_x86)
add_definitions (
             -Dcap_fixpoint_gen 
             -Dcap_noaddr_gen 
             -Dcap_stripped_binaries 
             -Dcap_tramp_liveness 
             -Dcap_virtual_registers
    )

elseif (PLATFORM MATCHES x86_64 OR PLATFORM MATCHES amd64)
add_definitions (-Darch_x86_64 -Darch_64bit)
add_definitions (
             -Dcap_32_64
             -Dcap_fixpoint_gen 
             -Dcap_noaddr_gen
             -Dcap_registers
             -Dcap_stripped_binaries 
             -Dcap_tramp_liveness 
    )

elseif (PLATFORM MATCHES ppc32)
add_definitions (-Darch_ppc32)
add_definitions (
             -Dcap_registers
    )

elseif (PLATFORM MATCHES ppc64)
add_definitions (-Darch_ppc64)
set (CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -m64)
set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -m64)
add_definitions (
             -Dcap_32_64
             -Dcap_registers
             -Dcap_toc_64
    )
endif (PLATFORM MATCHES i386)

if (PLATFORM MATCHES linux)
add_definitions (-Dos_linux)
add_definitions (
             -Dcap_async_events
             -Dcap_binary_rewriter
             -Dcap_dwarf
             -Dcap_mutatee_traps
             -Dcap_ptrace
    )
add_definitions (-Dbug_syscall_changepc_rewind -Dbug_force_terminate_failure)

elseif (PLATFORM MATCHES bgq_ion)
add_definitions (-Dos_bg -Dos_bgq -Dos_bgq_ion -Dos_linux)
add_definitions (
             -Dcap_async_events
             -Dcap_binary_rewriter
             -Dcap_dwarf
             -Dcap_mutatee_traps
             -Dcap_ptrace
    )
add_definitions (-Dbug_syscall_changepc_rewind)

elseif (PLATFORM MATCHES cnl)
add_definitions (-Dos_linux -Dos_cnl)
add_definitions (
             -Dcap_async_events
             -Dcap_binary_rewriter
             -Dcap_dwarf
             -Dcap_mutatee_traps
             -Dcap_ptrace
    )
add_definitions (-Dbug_syscall_changepc_rewind)

elseif (PLATFORM MATCHES freebsd)
add_definitions (-Dos_freebsd)
add_definitions (
             -Dcap_binary_rewriter
             -Dcap_dwarf
             -Dcap_mutatee_traps
    )
add_definitions (-Dbug_freebsd_missing_sigstop 
             -Dbug_freebsd_mt_suspend 
             -Dbug_freebsd_change_pc 
             -Dbug_phdrs_first_page 
             -Dbug_syscall_changepc_rewind
    )

elseif (PLATFORM MATCHES windows)
add_definitions (-Dos_windows)
add_definitions (
             -Dcap_mem_emulation
    )
endif (PLATFORM MATCHES linux)


if (PLATFORM STREQUAL i386-unknown-linux2.4)
add_definitions (-Di386_unknown_linux2_0)

elseif (PLATFORM STREQUAL x86_64-unknown-linux2.4)
add_definitions (-Dx86_64_unknown_linux2_4)

elseif (PLATFORM STREQUAL ppc32_linux)
add_definitions (-Dppc32_linux)
add_definitions (${BUG_DEF} -Dbug_registers_after_exit)

elseif (PLATFORM STREQUAL ppc64_linux)
add_definitions (-Dppc64_linux)
add_definitions (${BUG_DEF} -Dbug_registers_after_exit)

elseif (PLATFORM STREQUAL ppc64_bgq_ion)
add_definitions (-Dppc64_bluegene -Dppc64_linux)

elseif (PLATFORM STREQUAL x86_64_cnl)
add_definitions (-Dx86_64_cnl -Dx86_64_unknown_linux2_4)

elseif (PLATFORM STREQUAL i386-unknown-freebsd7.2)
add_definitions (-Di386_unknown_freebsd7_0)

elseif (PLATFORM STREQUAL amd64-unknown-freebsd7.2)
add_definitions (-Damd64_unknown_freebsd7_0)

else (PLATFORM STREQUAL i386-unknown-linux2.4)
  message (FATAL_ERROR "Unknown platform: $(PLATFORM)")
endif (PLATFORM STREQUAL i386-unknown-linux2.4)

if (HAVE_THREAD_DB)
add_definitions (-Dcap_thread_db)
endif (HAVE_THREAD_DB)


string (REPLACE ";" " " UNIFIED_DEF_STRING "${UNIFIED_DEF}") 

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${UNIFIED_DEF_STRING}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${UNIFIED_DEF_STRING}")

