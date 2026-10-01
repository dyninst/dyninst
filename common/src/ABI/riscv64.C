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
#include "registers/riscv64_regs.h"

namespace Dyninst { namespace abi {

  /*
   * RISC-V ELF psABI Specification
   * Version 1.0
   * November 2022
   *
   * Chapter 2 Procedure Calling Convention (LP64D)
   */
  definition make_riscv64() {
    definition d;

    d.function.params = register_set{
      riscv64::a0,  riscv64::a1,  riscv64::a2,  riscv64::a3,
      riscv64::a4,  riscv64::a5,  riscv64::a6,  riscv64::a7,
      riscv64::fa0, riscv64::fa1, riscv64::fa2, riscv64::fa3,
      riscv64::fa4, riscv64::fa5, riscv64::fa6, riscv64::fa7
    };

    d.function.returns = register_set{
      riscv64::a0,  riscv64::a1,
      riscv64::fa0, riscv64::fa1
    };

    d.function.preserved = register_set{
      riscv64::sp,
      riscv64::s0,  riscv64::s1,  riscv64::s2,  riscv64::s3,
      riscv64::s4,  riscv64::s5,  riscv64::s6,  riscv64::s7,
      riscv64::s8,  riscv64::s9,  riscv64::s10, riscv64::s11,
      riscv64::fs0, riscv64::fs1, riscv64::fs2, riscv64::fs3,
      riscv64::fs4, riscv64::fs5, riscv64::fs6, riscv64::fs7,
      riscv64::fs8, riscv64::fs9, riscv64::fs10, riscv64::fs11,
      // frm has thread storage duration, so it survives a call. fflags, which
      // shares the fcsr liveness bit, does not, but is not modelled separately.
      riscv64::fcsr
    };

    d.function.globals = register_set{
      riscv64::ra,  // return address
      riscv64::gp,  // global pointer
      riscv64::tp   // thread pointer
    };

    /*
     * Linux RISC-V system calls (ecall)
     *
     *   https://man7.org/linux/man-pages/man2/syscall.2.html
     *
     * The kernel restores every register except the return value in a0.
     */
    d.syscall.params = register_set{
      riscv64::a7,  // syscall number
      riscv64::a0,  riscv64::a1,  riscv64::a2,  riscv64::a3,  riscv64::a4,  riscv64::a5
    };

    d.syscall.returns = register_set{riscv64::a0};

    for(auto r : MachRegister::getAllRegistersForArch(Arch_riscv64)) {
      if(r.getBaseRegister() != riscv64::a0) {
        d.syscall.preserved.insert(r);
      }
    }

    return d;
  }

}}
