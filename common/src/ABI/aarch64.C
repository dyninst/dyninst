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
#include "registers/aarch64_regs.h"

namespace Dyninst { namespace abi {

  /*
   * Procedure Call Standard for the Arm 64-bit Architecture (AArch64)
   * April 12 2021
   *
   * Chapter 6 The Base Procedure Call Standard
   *
   * Registers are listed by their widest view (x<N>, q<N>); the narrower
   * views share its liveness bit.
   */
  definition make_aarch64() {
    definition d;

    d.function.params = register_set{
      aarch64::x0,  aarch64::x1,  aarch64::x2,  aarch64::x3,
      aarch64::x4,  aarch64::x5,  aarch64::x6,  aarch64::x7,
      aarch64::q0,  aarch64::q1,  aarch64::q2,  aarch64::q3,
      aarch64::q4,  aarch64::q5,  aarch64::q6,  aarch64::q7
    };

    d.function.returns = d.function.params;

    // The address of the memory for a returned composite (XR). The callee
    // need not preserve it.
    d.function.params.insert(aarch64::x8);

    d.function.preserved = register_set{
      aarch64::x19, aarch64::x20, aarch64::x21, aarch64::x22, aarch64::x23,
      aarch64::x24, aarch64::x25, aarch64::x26, aarch64::x27, aarch64::x28,
      aarch64::x29, // frame pointer
      aarch64::sp,
      // Only the bottom 64 bits of v8-v15 are preserved, but liveness does
      // not track partial registers.
      aarch64::q8,  aarch64::q9,  aarch64::q10, aarch64::q11,
      aarch64::q12, aarch64::q13, aarch64::q14, aarch64::q15,
      // The FPCR control fields (rounding mode, exception enables, ...) are
      // callee-saved, unless a function is documented to change them. The
      // cumulative exception flags in FPSR are not preserved.
      aarch64::fpcr
    };

    d.function.globals = register_set{
      // IP0 and IP1 may be used by call veneers and PLT stubs.
      aarch64::x16, aarch64::x17,
      // The platform register, if the platform uses one.
      aarch64::x18,
      // The link register.
      aarch64::x30
    };

    /*
     * Linux arm64 system calls (svc #0)
     *
     *   https://man7.org/linux/man-pages/man2/syscall.2.html
     *
     * The kernel restores every register except the return value in x0.
     */
    d.syscall.params = register_set{
      aarch64::x8,  // syscall number
      aarch64::x0,  aarch64::x1,  aarch64::x2,  aarch64::x3,  aarch64::x4,  aarch64::x5
    };

    d.syscall.returns = register_set{aarch64::x0};

    for(auto r : MachRegister::getAllRegistersForArch(Arch_aarch64)) {
      if(r.getBaseRegister() != aarch64::x0) {
        d.syscall.preserved.insert(r);
      }
    }

    return d;
  }

}}
