#
# -- Define the capabilities for each supported architecture/platform
#
#  cap_32_64 - This host 64-bit platform supports modifying 32-bit binaries
#

include_guard(GLOBAL)

## amdgpu special
if(DYNINST_ARCH_amdgpu)

  ## Only x86_64 for now, straight copy

  set(ARCH_DEFINES -Darch_amdgpu -Darch_64bit)
  set(CAP_DEFINES
      ${CAP_DEFINES} -Dcap_fixpoint_gen -Dcap_noaddr_gen -Dcap_registers
      -Dcap_tramp_liveness
      #-Dcap_stack_mods
      )

  ## amdgpu is 64 bit,no 32 bit arch
  #      -Dcap_32_64
  #      -Dcap_stripped_binaries

  #endif()
  ## Ignore the RT lib for now, chat with bart
  #  ## no dynamic lib on amdgpu
  #  set(CAP_DEFINES
  #      ${CAP_DEFINES}
  #	-DDYNINST_RT_STATIC_LIB
  #      )

  ## amdgpu special
else()

  set(CAP_DEFINES -Dcap_dynamic_heap -Dcap_liveness -Dcap_threads)

  if(DYNINST_HOST_ARCH_I386)
    set(ARCH_DEFINES_TESTSUITE -Darch_x86)
  elseif(DYNINST_HOST_ARCH_X86_64)
    set(ARCH_DEFINES_TESTSUITE -Darch_x86_64 -Darch_64bit)
  elseif(DYNINST_HOST_ARCH_PPC64LE)
    set(ARCH_DEFINES_TESTSUITE -Darch_power -Darch_64bit)
  elseif(DYNINST_HOST_ARCH_AARCH64)
    set(ARCH_DEFINES_TESTSUITE -Darch_aarch64 -Darch_64bit)
  endif()

  if(DYNINST_CODEGEN_ARCH_I386)
    set(ARCH_DEFINES_CODEGEN -Darch_x86)
    set(CAP_DEFINES
        ${CAP_DEFINES}
        -Dcap_fixpoint_gen
        -Dcap_noaddr_gen
        -Dcap_stripped_binaries
        -Dcap_tramp_liveness
        -Dcap_virtual_registers
        -Dcap_stack_mods)

  elseif(DYNINST_CODEGEN_ARCH_X86_64)
    set(ARCH_DEFINES_CODEGEN -Darch_x86_64 -Darch_64bit)
    set(CAP_DEFINES
        ${CAP_DEFINES}
        -Dcap_32_64
        -Dcap_fixpoint_gen
        -Dcap_noaddr_gen
        -Dcap_registers
        -Dcap_stripped_binaries
        -Dcap_tramp_liveness
        -Dcap_stack_mods)

  elseif(DYNINST_CODEGEN_ARCH_PPC64LE)
    set(ARCH_DEFINES_CODEGEN -Darch_power -Darch_64bit)
    set(CAP_DEFINES ${CAP_DEFINES} -Dcap_32_64 -Dcap_registers -Dcap_toc_64)
  elseif(DYNINST_CODEGEN_ARCH_AARCH64)
    set(ARCH_DEFINES_CODEGEN -Darch_aarch64 -Darch_64bit)
    set(CAP_DEFINES ${CAP_DEFINES} -Dcap_registers)
  elseif(DYNINST_CODEGEN_ARCH_RISCV64)
    set(ARCH_DEFINES_CODEGEN -Darch_riscv64 -Darch_64bit)
    set(CAP_DEFINES ${CAP_DEFINES} -Dcap_registers)
  elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
    set(ARCH_DEFINES_CODEGEN -Darch_amdgpu_gfx908 -Darch_64bit)
  elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX90A)
    set(ARCH_DEFINES_CODEGEN -Darch_amdgpu_gfx90a -Darch_64bit)
  elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX940)
    set(ARCH_DEFINES_CODEGEN -Darch_amdgpu_gfx940 -Darch_64bit)
  endif()
  ## amdgpu special
endif()

## OK, so this is a brute force copy again.

## amdgpu special
if(DYNINST_ARCH_amdgpu)
  ## need to control options more carefully
  ## Need to differentiate OS of target vs OS of host.
  ## Leave os_linux for now for host part of dyninst
  set(OS_DEFINES -Dos_linux)
  set(CAP_DEFINES ${CAP_DEFINES} -Dcap_async_events -Dcap_binary_rewriter -Dcap_dwarf
                  -Dcap_ptrace -Dcap_mutatee_traps)
  #	-Dcap_mutatee_traps
  set(BUG_DEFINES -Dbug_syscall_changepc_rewind -Dbug_force_terminate_failure)
  ## might need to suck in -Dwhatever defines here
  ## I was trying to cooperate, but too much stuff

  ## I"m doing to leave out the -D<arch>[_unknown]_linux[0-9_]+  for
  ## now to see what else breaks

  ## amdgpu special (was if)
elseif(DYNINST_OS_Linux)
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

string(REGEX REPLACE "-D" "-DDYNINST_HOST_" _LOWER_ARCH_DEFINES
                     "${ARCH_DEFINES_TESTSUITE}")
if(NOT DYNINST_ARCH_amdgpu)
  string(TOUPPER "${_LOWER_ARCH_DEFINES}" ARCH_DEFINES)
endif()

string(REGEX REPLACE "-D" "-DDYNINST_CODEGEN_" ARCH_DEFINES_CODEGEN
                     "${ARCH_DEFINES_CODEGEN}")
string(TOUPPER "${ARCH_DEFINES_CODEGEN}" ARCH_DEFINES_CODEGEN)

set(DYNINST_PLATFORM_CAPABILITIES ${CAP_DEFINES} ${BUG_DEFINES} ${ARCH_DEFINES}
                                  ${OS_DEFINES} ${OLD_DEFINES} ${ARCH_DEFINES_CODEGEN})

# The testsuite assumes the architecture defines look like `arch_x86_64`
# so we need to keep that format.

set(TESTSUITE_PLATFORM_CAPABILITIES
    ${CAP_DEFINES} ${BUG_DEFINES} ${ARCH_DEFINES_TESTSUITE} ${OS_DEFINES} ${OLD_DEFINES})
