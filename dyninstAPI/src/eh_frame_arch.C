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

#include "eh_frame_arch.h"

#include "registers/x86_64_regs.h"
#include "registers/aarch64_regs.h"
#include "registers/ppc64_regs.h"

namespace Dyninst {

const EHFrameArch* ehFrameArchFor(Architecture arch) {
  // x86-64: the return address is kept on the stack at CFA-8 via a constant
  // rule, so it is not sampled. Initial CFI: def_cfa rsp(7),8 ; ra(16) @ cfa-8.
  static const EHFrameArch k_x86_64 = {
    /* returnAddrReg       */ 16,
    /* cieInitialCFI       */ { 0x0c, 7, 8, 0x90, 1 },
    /* calleeSavedDwarfRegs*/ { 3, 6, 12, 13, 14, 15 },   // rbx, rbp, r12-r15
    /* framePtr            */ x86_64::rbp,
    /* framePtrDwarf       */ 6,
    /* stackPtrDwarf       */ 7,
  };

  // aarch64: link-register architecture. lr(x30) is sampled like any other
  // callee-saved register. Initial CFI: def_cfa sp(31),0.
  static const EHFrameArch k_aarch64 = {
    /* returnAddrReg       */ 30,
    /* cieInitialCFI       */ { 0x0c, 31, 0 },
    /* calleeSavedDwarfRegs*/ { 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 },  // x19-x28, fp, lr
    /* framePtr            */ aarch64::x29,
    /* framePtrDwarf       */ 29,
    /* stackPtrDwarf       */ 31,
  };

  // ppc64le (ELFv2): link-register architecture. The link register is DWARF
  // number 65 and is sampled like a callee-saved register. Initial CFI:
  // def_cfa r1(sp),0.
  static const EHFrameArch k_ppc64 = {
    /* returnAddrReg       */ 65,
    /* cieInitialCFI       */ { 0x0c, 1, 0 },
    /* calleeSavedDwarfRegs*/ { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                24, 25, 26, 27, 28, 29, 30, 31, 65 },  // r14-r31, lr(65)
    /* framePtr            */ ppc64::r31,
    /* framePtrDwarf       */ 31,
    /* stackPtrDwarf       */ 1,
  };

  switch (arch) {
    case Arch_x86_64:  return &k_x86_64;
    case Arch_aarch64: return &k_aarch64;
    case Arch_ppc64:   return &k_ppc64;
    default:           return nullptr;
  }
}

}  // namespace Dyninst
