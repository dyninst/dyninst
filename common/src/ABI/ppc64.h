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

#ifndef DYNINST_COMMON_PPC64_ABI_H
#define DYNINST_COMMON_PPC64_ABI_H

#include "registers/ppc64_regs.h"
#include "registers/registerSet.h"
#include "ABI/architecture.h"
#include "Architecture.h"
#include "registers/MachRegister.h"

namespace Dyninst { namespace abi {

/*
 * OpenPOWER 64-Bit ELF V2 ABI Specification
 * Version 1.5
 * December 1, 2020
 *
 * Section 2.2 Function Calling Sequence
 *
 * While it is recommended that all functions use the standard calling sequence,
 * the requirements of the standard calling sequence are only applicable to
 * global functions. Different calling sequences and conventions can be used by
 * local functions that cannot be reached from other compilation units, if they
 * comply with the stack back trace requirements. Some tools may not work with
 * alternate calling sequences and conventions.
 */

architecture make_ppc64() {
  architecture arch;
  arch.address_width = getArchAddressWidth(Arch_ppc64);

  arch.function.params = {
    ppc64::r3,    // GPRs
    ppc64::r4,
    ppc64::r5,
    ppc64::r6,
    ppc64::r7,
    ppc64::r8,
    ppc64::r9,
    ppc64::r10,
    ppc64::fpr1,  // Floating-point registers
    ppc64::fpr2,
    ppc64::fpr3,
    ppc64::fpr4,
    ppc64::fpr5,
    ppc64::fpr6,
    ppc64::fpr7,
    ppc64::fpr8,
    ppc64::fpr9,
    ppc64::fpr10,
    ppc64::fpr11,
    ppc64::fpr12,
    ppc64::fpr13,
    ppc64::vsr2,  // Vector-scalar registers
    ppc64::vsr3,
    ppc64::vsr4,
    ppc64::vsr5,
    ppc64::vsr6,
    ppc64::vsr7,
    ppc64::vsr8,
    ppc64::vsr9,
    ppc64::vsr10,
    ppc64::vsr11,
    ppc64::vsr12,
    ppc64::vsr13,
  };

  arch.function.returns = {
    ppc64::r3,    // GPRs
    ppc64::r4,
    ppc64::r5,
    ppc64::r6,
    ppc64::r7,
    ppc64::r8,
    ppc64::r9,
    ppc64::r10,
    ppc64::fpr1,  // Floating-point registers
    ppc64::fpr2,
    ppc64::fpr3,
    ppc64::fpr4,
    ppc64::fpr5,
    ppc64::fpr6,
    ppc64::fpr7,
    ppc64::fpr8,
    ppc64::fpr9,
    ppc64::fpr10,
    ppc64::fpr11,
    ppc64::fpr12,
    ppc64::fpr13,
    ppc64::vsr2,  // Vector-scalar registers
    ppc64::vsr3,
    ppc64::vsr4,
    ppc64::vsr5,
    ppc64::vsr6,
    ppc64::vsr7,
    ppc64::vsr8,
    ppc64::vsr9,
    ppc64::vsr10,
    ppc64::vsr11,
    ppc64::vsr12,
    ppc64::vsr13,
  };

  arch.function.preserved = {
    ppc64::r1,    // stack pointer
    ppc64::r2,    // TOC pointer (only saved between calls in same compilation unti)
    ppc64::r14,   // GPRs for local variables (nonvolatile)
    ppc64::r15,
    ppc64::r16,
    ppc64::r17,
    ppc64::r18,
    ppc64::r19,
    ppc64::r20,
    ppc64::r21,
    ppc64::r22,
    ppc64::r23,
    ppc64::r24,
    ppc64::r25,
    ppc64::r26,
    ppc64::r27,
    ppc64::r28,
    ppc64::r29,
    ppc64::r30,
    ppc64::r31,
    ppc64::cr2,   // Condition register fields
    ppc64::cr3,
    ppc64::cr4,
//    DSCR (ppc64::dsisr?)    // Data stream prefect control
    ppc64::fpr14, // Floating-point registers for local variables
    ppc64::fpr15,
    ppc64::fpr16,
    ppc64::fpr17,
    ppc64::fpr18,
    ppc64::fpr19,
    ppc64::fpr20,
    ppc64::fpr21,
    ppc64::fpr22,
    ppc64::fpr23,
    ppc64::fpr24,
    ppc64::fpr25,
    ppc64::fpr26,
    ppc64::fpr27,
    ppc64::fpr28,
    ppc64::fpr29,
    ppc64::fpr30,
    ppc64::fpr31,
    ppc64::vsr20, // Vector-scalar registers for local variables
    ppc64::vsr21,
    ppc64::vsr22,
    ppc64::vsr23,
    ppc64::vsr24,
    ppc64::vsr25,
    ppc64::vsr26,
    ppc64::vsr27,
    ppc64::vsr28,
    ppc64::vsr29,
    ppc64::vsr30,
    ppc64::vsr31,
  };

  arch.function.globals = {
    ppc64::r12,     // Optional use in function linkage
    ppc64::r13,     // Thread pointer (reserved use)
//    TAR    // Reserved for system use
    ppc64::vrsave,  // Reserved for system use
//    FPSCR see 2.2.2.2 Limited-Access bits
//    VSCR see 2.2.2.2 Limited-Access bits
  };

  /*
   * Power Architecture 64-bit Linux system call ABI (sc instruction)
   *
   *   Linux v6.8 Documentation/arch/powerpc/syscall64-abi.rst
   *   https://github.com/torvalds/linux/blob/v6.8/Documentation/arch/powerpc/syscall64-abi.rst
   *
   * The syscall number is in r0 and up to six arguments are passed in r3-r8.
   * r3 holds the return value and cr0.SO is set when the call fails.
   *
   * Register preservation matches the ELF v2 ABI calling sequence with these
   * differences: r0, r3-r8 and cr0 are volatile, while lr and cr1/cr5-cr7 are
   * preserved (they are volatile in the function calling sequence).
   */
  arch.syscall.params = {
    ppc64::r0,   // syscall number
    ppc64::r3,   // arg1
    ppc64::r4,   // arg2
    ppc64::r5,   // arg3
    ppc64::r6,   // arg4
    ppc64::r7,   // arg5
    ppc64::r8    // arg6
  };

  arch.syscall.returns = {
    ppc64::r3    // return value (cr0.SO signals an error)
  };

  // Start from the function-call preserved set and add the registers that the
  // syscall sequence preserves but the function sequence does not.
  arch.syscall.preserved = arch.function.preserved;
  arch.syscall.preserved |= {
    ppc64::r13,   // thread pointer (dedicated, nonvolatile)
    ppc64::lr,    // sc preserves the link register
    ppc64::cr1,   // sc preserves cr1 and cr5-cr7; only cr0 is volatile
    ppc64::cr5,
    ppc64::cr6,
    ppc64::cr7
  };

  arch.syscall.globals = {};

  return arch;
}

}}

#endif
