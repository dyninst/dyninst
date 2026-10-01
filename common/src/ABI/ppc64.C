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
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"

namespace Dyninst { namespace abi {

  /*
   * OpenPOWER 64-Bit ELF V2 ABI Specification
   * Version 1.5
   * December 1, 2020
   *
   * Section 2.2 Function Calling Sequence
   */
  definition make_ppc64() {
    definition d;

    // The instruction decoder still reports most PowerPC registers in the
    // ppc32 namespace, so liveness results are reported there too. Registers
    // from either namespace are accepted.
    //
    // The vector registers below have no ppc32 name, so they get no liveness
    // bit and are not tracked. Dyninst numbers them vsr0-vsr31 (fpr<N> is
    // vsr<32+N>), so vsr2 here is the ABI's v2, not VSR2.
    d.register_namespace = Arch_ppc32;

    d.function.params = register_set{
      ppc64::r3,    ppc64::r4,    ppc64::r5,    ppc64::r6,
      ppc64::r7,    ppc64::r8,    ppc64::r9,    ppc64::r10,
      ppc64::r12,   // function entry address, for global entry points
      ppc64::fpr1,  ppc64::fpr2,  ppc64::fpr3,  ppc64::fpr4,  ppc64::fpr5,
      ppc64::fpr6,  ppc64::fpr7,  ppc64::fpr8,  ppc64::fpr9,  ppc64::fpr10,
      ppc64::fpr11, ppc64::fpr12, ppc64::fpr13,
      ppc64::vsr2,  ppc64::vsr3,  ppc64::vsr4,  ppc64::vsr5,  ppc64::vsr6,
      ppc64::vsr7,  ppc64::vsr8,  ppc64::vsr9,  ppc64::vsr10, ppc64::vsr11,
      ppc64::vsr12, ppc64::vsr13
    };

    d.function.returns = register_set{
      ppc64::r3,    ppc64::r4,    ppc64::r5,    ppc64::r6,
      ppc64::r7,    ppc64::r8,    ppc64::r9,    ppc64::r10,
      ppc64::fpr1,  ppc64::fpr2,  ppc64::fpr3,  ppc64::fpr4,  ppc64::fpr5,
      ppc64::fpr6,  ppc64::fpr7,  ppc64::fpr8,  ppc64::fpr9,  ppc64::fpr10,
      ppc64::fpr11, ppc64::fpr12, ppc64::fpr13,
      ppc64::vsr2,  ppc64::vsr3,  ppc64::vsr4,  ppc64::vsr5,  ppc64::vsr6,
      ppc64::vsr7,  ppc64::vsr8,  ppc64::vsr9,  ppc64::vsr10, ppc64::vsr11,
      ppc64::vsr12, ppc64::vsr13
    };

    d.function.preserved = register_set{
      ppc64::r1,    // stack pointer
      ppc64::r2,    // TOC pointer
      ppc64::r13,   // thread pointer
      ppc64::r14,   ppc64::r15,   ppc64::r16,   ppc64::r17,   ppc64::r18,
      ppc64::r19,   ppc64::r20,   ppc64::r21,   ppc64::r22,   ppc64::r23,
      ppc64::r24,   ppc64::r25,   ppc64::r26,   ppc64::r27,   ppc64::r28,
      ppc64::r29,   ppc64::r30,   ppc64::r31,
      // Every CR field shares the single CR liveness bit, so listing these
      // treats the whole CR as preserved: cr0, cr1 and cr5-cr7 are never
      // treated as clobbered by a call.
      ppc64::cr2,   ppc64::cr3,   ppc64::cr4,
      ppc64::fpr14, ppc64::fpr15, ppc64::fpr16, ppc64::fpr17, ppc64::fpr18,
      ppc64::fpr19, ppc64::fpr20, ppc64::fpr21, ppc64::fpr22, ppc64::fpr23,
      ppc64::fpr24, ppc64::fpr25, ppc64::fpr26, ppc64::fpr27, ppc64::fpr28,
      ppc64::fpr29, ppc64::fpr30, ppc64::fpr31,
      ppc64::vsr20, ppc64::vsr21, ppc64::vsr22, ppc64::vsr23, ppc64::vsr24,
      ppc64::vsr25, ppc64::vsr26, ppc64::vsr27, ppc64::vsr28, ppc64::vsr29,
      ppc64::vsr30, ppc64::vsr31,
      ppc64::fpscw    // only its control fields, which are not modelled separately
    };

    d.function.globals = register_set{
      ppc64::vrsave   // reserved for system use
    };

    /*
     * Linux 64-bit PowerPC system calls (the sc instruction)
     *
     *   https://github.com/torvalds/linux/blob/v6.8/Documentation/arch/powerpc/syscall64-abi.rst
     *
     * The rules are the function-call rules, except that lr, cr1 and cr5-cr7
     * are preserved, and that "all floating point and vector data registers as
     * well as control and status registers are nonvolatile". That leaves r0,
     * r3-r12, ctr, xer and cr0 volatile. cr0 shares the CR liveness bit with
     * the preserved fields, so the CR is treated as preserved.
     */
    d.syscall.params = register_set{
      ppc64::r0,    // syscall number
      ppc64::r3,    ppc64::r4,    ppc64::r5,    ppc64::r6,    ppc64::r7,    ppc64::r8
    };

    d.syscall.returns = register_set{ppc64::r3};

    register_set const clobbered{
      ppc32::r0,    ppc32::r3,    ppc32::r4,    ppc32::r5,    ppc32::r6,
      ppc32::r7,    ppc32::r8,    ppc32::r9,    ppc32::r10,   ppc32::r11,
      ppc32::r12,   ppc32::ctr,   ppc32::xer
    };
    for(auto r : MachRegister::getAllRegistersForArch(Arch_ppc32)) {
      if(!clobbered.contains(r.getBaseRegister())) {
        d.syscall.preserved.insert(r);
      }
    }

    return d;
  }

}}
