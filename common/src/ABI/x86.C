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
#include "registers/x86_regs.h"

namespace Dyninst { namespace abi {

  /*
   * System V Application Binary Interface
   * Intel386 Architecture Processor Supplement
   * Version 1.0
   * February 3, 2015
   *
   * Section 2.2 Function Calling Sequence
   *
   * Vector registers are listed by their xmm view; the ymm and zmm views of
   * the same register share its liveness bit.
   */
  definition make_x86() {
    definition d;

    d.function.params = register_set{
      x86::xmm0, x86::xmm1, x86::xmm2,  // __m128 and __m256
      x86::mm0,  x86::mm1,  x86::mm2,   // __m64
      x86::eax,  x86::edx,  x86::ecx    // regparm(3); fastcall uses ecx and edx
    };

    d.function.returns = register_set{
      x86::eax,   // integers, pointers, address of a returned struct
      x86::edx,   // upper half of 64-bit integers
      x86::xmm0,  // __m128 and __m256
      x86::mm0,   // __m64
      x86::st0,   // float, double, long double
      x86::st1    // imaginary part of a complex float, double or long double
    };

    d.function.preserved = register_set{
      x86::ebx,  x86::esp,  x86::ebp,  x86::esi,  x86::edi,
      x86::mxcsr,  // only its control bits, which are not modelled separately
      x86::fcw     // x87 control word
    };

    // The direction flag must also be clear on entry and return, but it is not
    // listed: it shares the liveness bit of the arithmetic flags, which would
    // then stay live across every call and return.
    d.function.globals = register_set{
      x86::gs,   // thread-specific data
      x86::ebx   // GOT pointer for calls through the PLT
    };

    /*
     * Linux `int 0x80` system calls
     *
     *   https://github.com/torvalds/linux/blob/v6.8/arch/x86/entry/entry_32.S (entry_INT80_32)
     *   https://man7.org/linux/man-pages/man2/syscall.2.html
     *
     * The kernel saves the user registers on entry and restores all of them
     * except eax, which holds the return value.
     */
    d.syscall.params = register_set{
      x86::eax,  // syscall number
      x86::ebx,  x86::ecx,  x86::edx,  x86::esi,  x86::edi,  x86::ebp
    };

    d.syscall.returns = register_set{x86::eax};

    for(auto r : MachRegister::getAllRegistersForArch(Arch_x86)) {
      if(r.getBaseRegister() != x86::eax) {
        d.syscall.preserved.insert(r);
      }
    }

    return d;
  }

}}
