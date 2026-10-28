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

#include "ABI/definitions.h"
#include "registers/x86_64_regs.h"

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
   *
   * Vector registers are listed by their xmm view; the ymm and zmm views of
   * the same register share its liveness bit.
   */
  definition make_x86_64() {
    definition d;

    d.function.params = register_set{
      x86_64::rax,  // number of vector registers used by a varargs call
      x86_64::r10,  // static chain pointer
      x86_64::rdi,  x86_64::rsi,  x86_64::rdx,  x86_64::rcx,  x86_64::r8,   x86_64::r9,
      x86_64::xmm0, x86_64::xmm1, x86_64::xmm2, x86_64::xmm3,
      x86_64::xmm4, x86_64::xmm5, x86_64::xmm6, x86_64::xmm7
    };

    d.function.returns = register_set{
      x86_64::rax,  x86_64::rdx,
      x86_64::xmm0, x86_64::xmm1,
      x86_64::st0,  x86_64::st1   // long double
    };

    d.function.preserved = register_set{
      x86_64::rbx,  x86_64::rsp,  x86_64::rbp,
      x86_64::r12,  x86_64::r13,  x86_64::r14,  x86_64::r15,
      x86_64::fs,     // thread control block
      x86_64::mxcsr,  // only its control bits, which are not modelled separately
      x86_64::fcw     // x87 control word
    };

    // The direction flag must also be clear on entry and return, but it is not
    // listed as an ABI global: it shares the liveness bit of the arithmetic
    // flags, which would then stay live across every call and return.

    /*
     * A.2 AMD64 Linux Kernel Conventions
     *
     * The kernel preserves every register except the return value in %rax.
     * The syscall instruction itself destroys %rcx (return address) and %r11
     * (saved RFLAGS).
     */
    d.syscall.params = register_set{
      x86_64::rax,  // syscall number
      x86_64::rdi,  x86_64::rsi,  x86_64::rdx,  x86_64::r10,  x86_64::r8,   x86_64::r9
    };

    d.syscall.returns = register_set{x86_64::rax};

    register_set const clobbered{x86_64::rax, x86_64::rcx, x86_64::r11};
    for(auto r : MachRegister::getAllRegistersForArch(Arch_x86_64)) {
      if(!clobbered.contains(r.getBaseRegister())) {
        d.syscall.preserved.insert(r);
      }
    }

    return d;
  }

}}
