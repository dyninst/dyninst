#
# -- Define the capabilities for each supported architecture/platform
#
#  cap_32_64 - This host 64-bit platform supports modifying 32-bit binaries
#

include_guard(GLOBAL)

set(CAP_DEFINES -Dcap_dynamic_heap -Dcap_liveness -Dcap_threads)

if(DYNINST_HOST_ARCH_I386)
  set(ARCH_DEFINES -DDYNINST_HOST_ARCH_X86)
  set(CAP_DEFINES
      ${CAP_DEFINES}
      -Dcap_fixpoint_gen
      -Dcap_noaddr_gen
      -Dcap_stripped_binaries
      -Dcap_tramp_liveness
      -Dcap_virtual_registers
      -Dcap_stack_mods)

elseif(DYNINST_HOST_ARCH_X86_64)
  set(ARCH_DEFINES -DDYNINST_HOST_ARCH_X86_64 -DDYNINST_HOST_ARCH_64BIT)
  set(CAP_DEFINES
      ${CAP_DEFINES}
      -Dcap_32_64
      -Dcap_fixpoint_gen
      -Dcap_noaddr_gen
      -Dcap_registers
      -Dcap_stripped_binaries
      -Dcap_tramp_liveness
      -Dcap_stack_mods)

elseif(DYNINST_HOST_ARCH_PPC64LE)
  set(ARCH_DEFINES -DDYNINST_HOST_ARCH_POWER -DDYNINST_HOST_ARCH_64BIT)
  set(CAP_DEFINES ${CAP_DEFINES} -Dcap_32_64 -Dcap_registers -Dcap_toc_64)

elseif(DYNINST_HOST_ARCH_AARCH64)
  set(ARCH_DEFINES -DDYNINST_HOST_ARCH_AARCH64 -DDYNINST_HOST_ARCH_64BIT)
  set(CAP_DEFINES ${CAP_DEFINES} -Dcap_registers)
endif()

if(DYNINST_OS_Linux)
  set(OS_DEFINES -Dos_linux)
  set(CAP_DEFINES ${CAP_DEFINES} -Dcap_async_events -Dcap_binary_rewriter -Dcap_dwarf
                  -Dcap_mutatee_traps -Dcap_ptrace)
  set(BUG_DEFINES -Dbug_syscall_changepc_rewind -Dbug_force_terminate_failure)

  if(DYNINST_HOST_ARCH_I386)
    set(OLD_DEFINES -Di386_unknown_linux2_0)
  elseif(DYNINST_HOST_ARCH_X86_64)
    set(OLD_DEFINES -Dx86_64_unknown_linux2_4)
  elseif(DYNINST_HOST_ARCH_PPC64LE)
    set(OLD_DEFINES -Dppc64_linux)
    set(BUG_DEFINES ${BUG_DEFINES} -Dbug_registers_after_exit)
  elseif(DYNINST_HOST_ARCH_AARCH64)
    set(OLD_DEFINES -Daarch64_unknown_linux)
  endif()

elseif(DYNINST_OS_FreeBSD)
  set(OS_DEFINES -Dos_freebsd)
  set(CAP_DEFINES ${CAP_DEFINES} -Dcap_binary_rewriter -Dcap_dwarf -Dcap_mutatee_traps)
  set(BUG_DEFINES
      -Dbug_freebsd_missing_sigstop -Dbug_freebsd_mt_suspend -Dbug_freebsd_change_pc
      -Dbug_phdrs_first_page -Dbug_syscall_changepc_rewind)

  if(DYNINST_HOST_ARCH_I386)
    set(OLD_DEFINES -Di386_unknown_freebsd7_0)
  elseif(DYNINST_HOST_ARCH_X86_64)
    set(OLD_DEFINES -Damd64_unknown_freebsd7_0)
  endif()

elseif(DYNINST_OS_Windows)
  set(OS_DEFINES -Dos_windows)
  set(CAP_DEFINES ${CAP_DEFINES} -Dcap_mutatee_traps)
  set(OLD_DEFINES -Di386_unknown_nt4_0)
endif()

set(DYNINST_PLATFORM_CAPABILITIES ${CAP_DEFINES} ${BUG_DEFINES} ${ARCH_DEFINES}
                                  ${OS_DEFINES} ${OLD_DEFINES})

# The testsuite assumes the architecture defines look like `arch_x86_64`
# so we need to keep that format.

string(TOLOWER "${ARCH_DEFINES}" _LOWER_ARCH_DEFINES)
string(REGEX REPLACE "dyninst_host_" "" _ARCH_DEFINES_TESTSUITE "${_LOWER_ARCH_DEFINES}")

set(TESTSUITE_PLATFORM_CAPABILITIES
    ${CAP_DEFINES} ${BUG_DEFINES} ${_ARCH_DEFINES_TESTSUITE} ${OS_DEFINES} ${OLD_DEFINES})
