/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DYNINST_COMMON_X86_64_ABI_H
#define DYNINST_COMMON_X86_64_ABI_H

#include "registers/x86_64_regs.h"
#include "registers/registerSet.h"
#include "ABI/architecture.h"
#include "Architecture.h"
#include "registers/MachRegister.h"

namespace Dyninst { namespace abi {

/*
 * System V Application Binary Interface
 * AMD64 Architecture Processor Supplement
 * (With LP64 and ILP32 Programming Models)
 * Version 1.0
 * November 7, 2023
 *
 * Section 3.2 Function Calling Sequence
 * Figure 3.4: Register Usage
 */

architecture make_x86_64() {
  architecture arch;
  arch.address_width = getArchAddressWidth(Arch_x86_64);

  arch.function.params = {
    x86_64::rax,   // number of vector registers for varargs
    x86_64::rdi,
    x86_64::rsi,
    x86_64::rdx,
    x86_64::rcx,
    x86_64::r8,
    x86_64::r9,
    x86_64::xmm0,   // __m128
    x86_64::xmm1,
    x86_64::xmm2,
    x86_64::xmm3,
    x86_64::xmm4,
    x86_64::xmm5,
    x86_64::xmm6,
    x86_64::xmm7,
    x86_64::ymm0,   // __m256
    x86_64::ymm1,
    x86_64::ymm2,
    x86_64::ymm3,
    x86_64::ymm4,
    x86_64::ymm5,
    x86_64::ymm6,
    x86_64::ymm7,
    x86_64::zmm0,   // __m512
    x86_64::zmm1,
    x86_64::zmm2,
    x86_64::zmm3,
    x86_64::zmm4,
    x86_64::zmm5,
    x86_64::zmm6,
    x86_64::zmm7
  };

  arch.function.returns = {
    x86_64::rax,
    x86_64::rdx,
    x86_64::xmm0,   // __m128
    x86_64::xmm1,
    x86_64::ymm0,   // __m256
    x86_64::ymm1,
    x86_64::zmm0,   // __m512
    x86_64::zmm1,
    x86_64::st0,    // long double
    x86_64::st1
  };

  arch.function.preserved = {
    x86_64::rbx,
    x86_64::rsp,
    x86_64::rbp,    // can be used as frame pointer
    x86_64::r12,
    x86_64::r13,
    x86_64::r14,
    x86_64::r15,    // optionally used as GOT base pointer
    x86_64::fs,     // Thread control block
    x86_64::mxcsr,  // only control bits, but we don't break it down
    x86_64::fcw     // x87 control word
  };

  arch.function.globals = {
    /*
    * The direction flag DF in the %RFLAGS register must be clear (set to “forward”
    * direction) on function entry and return. Other user flags have no specified role
    * in the standard calling sequence and are not preserved across calls.
    */
    x86_64::df,
    x86_64::r10   // Used for passing a function’s static chain pointer
  };

  /*
   * A.2 AMD64 Linux Kernel Conventions
   */

  // User-level applications (UIA) and the kernel interface (KI) use overlapping
  // sets of registers to pass arguments.
  arch.syscall.params = {
    x86_64::rax,  // syscall number
    x86_64::rdi,  // arg1: UIA, KI
    x86_64::rsi,  // arg2: UIA, KI
    x86_64::rdx,  // arg3: UIA, KI
    x86_64::rcx,  // arg4: UIA
    x86_64::r10,  // arg4: KI
    x86_64::r8,   // arg5: UIA, KI
    x86_64::r9,   // arg6: UIA, KI
  };

  arch.syscall.returns = {
    x86_64::rax   // The result of the system-call
  };

  // The kernel clobbers %rcx and %r11 but preserves all others except %rax.
  auto const& all_regs = MachRegister::getAllRegistersForArch(Arch_x86_64);
  for(auto r : all_regs) {
    arch.syscall.preserved.insert(r);
  }
  arch.syscall.preserved.remove(x86_64::rax);

  return arch;
}

}}

#endif
