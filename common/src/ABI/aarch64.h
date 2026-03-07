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

#ifndef DYNINST_COMMON_AARCH64_ABI_H
#define DYNINST_COMMON_AARCH64_ABI_H

#include "registers/aarch64_regs.h"
#include "registers/registerSet.h"
#include "ABI/architecture.h"
#include "Architecture.h"
#include "registers/MachRegister.h"

namespace Dyninst { namespace abi {

/*
 * Procedure Call Standard for the Arm 64-bit Architecture (AArch64)
 * April 12 2021
 *
 * Chapter 6 The Base Procedure Call Standard
 */

architecture make_aarch64() {
  architecture arch;
  arch.address_width = getArchAddressWidth(Arch_aarch64);

  arch.function.params = {
    aarch64::x0,  // 32- and 64-bit GPRs
    aarch64::w0,
    aarch64::x1,
    aarch64::w1,
    aarch64::x2,
    aarch64::w2,
    aarch64::x3,
    aarch64::w3,
    aarch64::x4,
    aarch64::w4,
    aarch64::x5,
    aarch64::w5,
    aarch64::x6,
    aarch64::w6,
    aarch64::x7,
    aarch64::w7,
    aarch64::q0,  // 128-, 64-, and 32-bit SIMD
    aarch64::d0,
    aarch64::s0,
    aarch64::q1,
    aarch64::d1,
    aarch64::s1,
    aarch64::q2,
    aarch64::d2,
    aarch64::s2,
    aarch64::q3,
    aarch64::d3,
    aarch64::s3,
    aarch64::q4,
    aarch64::d4,
    aarch64::s4,
    aarch64::q5,
    aarch64::d5,
    aarch64::s5,
    aarch64::q6,
    aarch64::d6,
    aarch64::s6,
    aarch64::q7,
    aarch64::d7,
    aarch64::s7
  };

  arch.function.returns = {
    aarch64::x0,  // 32- and 64-bit GPRs
    aarch64::w0,
    aarch64::x1,
    aarch64::w1,
    aarch64::x2,
    aarch64::w2,
    aarch64::x3,
    aarch64::w3,
    aarch64::x4,
    aarch64::w4,
    aarch64::x5,
    aarch64::w5,
    aarch64::x6,
    aarch64::w6,
    aarch64::x7,
    aarch64::w7,
    aarch64::x8,  // Indirect result location register
    aarch64::w8,
    aarch64::q0,  // 128-, 64-, and 32-bit SIMD
    aarch64::d0,
    aarch64::s0,
    aarch64::q1,
    aarch64::d1,
    aarch64::s1,
    aarch64::q2,
    aarch64::d2,
    aarch64::s2,
    aarch64::q3,
    aarch64::d3,
    aarch64::s3,
    aarch64::q4,
    aarch64::d4,
    aarch64::s4,
    aarch64::q5,
    aarch64::d5,
    aarch64::s5,
    aarch64::q6,
    aarch64::d6,
    aarch64::s6,
    aarch64::q7,
    aarch64::d7,
    aarch64::s7
  };

  arch.function.preserved = {
    // All 64 bits of each value stored in r19-r29 must be preserved, even
    // when using the ILP32 data model.
    aarch64::x19,
    aarch64::w19,
    aarch64::x20,
    aarch64::w20,
    aarch64::x21,
    aarch64::w21,
    aarch64::x22,
    aarch64::w22,
    aarch64::x23,
    aarch64::w23,
    aarch64::x24,
    aarch64::w24,
    aarch64::x25,
    aarch64::w25,
    aarch64::x26,
    aarch64::w26,
    aarch64::x27,
    aarch64::w27,
    aarch64::x28,
    aarch64::w28,
    aarch64::sp,    // stack pointer

    // 128-, 64-, and 32-bit SIMD
    // Only the bottom 64 bits of each value stored in v8-v15 need
    // to be preserved, but we mark the whole register.
    aarch64::q8,
    aarch64::d8,
    aarch64::s8,
    aarch64::q9,
    aarch64::d9,
    aarch64::s9,
    aarch64::q10,
    aarch64::d10,
    aarch64::s10,
    aarch64::q11,
    aarch64::d11,
    aarch64::s11,
    aarch64::q12,
    aarch64::d12,
    aarch64::s12,
    aarch64::q13,
    aarch64::d13,
    aarch64::s13,
    aarch64::q14,
    aarch64::d14,
    aarch64::s14,
    aarch64::q15,
    aarch64::d15,
    aarch64::s15
  };

  arch.function.globals = {
    // r16/IP0 and r17/IP1 can be used by call veneers and PLT code, but may
    // also be a temporary. Conservatively assume they are always used.
    aarch64::x16,
    aarch64::w16,
    aarch64::x17,
    aarch64::w17,

    /* The Platform Register, if needed; otherwise a temporary register
     *
     * Software developers creating platform-independent code are advised to
     * avoid using r18 if at all possible. It should not be assumed that treating
     * the register as callee-saved will be sufficient to satisfy the requirements
     * of the platform.
     */
    aarch64::x18,
    aarch64::w18,

    aarch64::x30, // Link register (LR)
    aarch64::w30,
    aarch64::x29, // Frame pointer (FP)
    aarch64::w29

    // Note: The N, Z, C and V flags are undefined on entry to and return
    //       from a public interface. We don't need to do anything with them.
  };

  arch.syscall.params = {};
  arch.syscall.returns = {};
  arch.syscall.preserved = {};
  arch.syscall.globals = {};

  return arch;
}

}}

#endif
