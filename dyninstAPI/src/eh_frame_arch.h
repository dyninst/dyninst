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

#ifndef DYNINST_EH_FRAME_ARCH_H
#define DYNINST_EH_FRAME_ARCH_H

#include <vector>

#include "Architecture.h"
#include "registers/MachRegister.h"

namespace Dyninst {

// Per-architecture parameters for synthesizing DWARF CFI (.eh_frame) for
// relocated code.
//
// The synthesis itself -- sampling the original frame rules via libdw, laying
// out CIEs/FDEs, and regenerating the .gcc_except_table -- is architecture
// neutral. Only three things differ per architecture:
//   * the CIE return_address_register,
//   * the CIE's initial CFI (the CFA definition and, on architectures that
//     keep the return address on the stack at a fixed offset, the constant
//     rule that recovers it), and
//   * the set of callee-saved registers whose rules are sampled per PC.
//
// On link-register architectures (aarch64, ppc64le) the return address is held
// in a register that is spilled like any other callee-saved register, so it is
// sampled per PC and appears in `calleeSavedDwarfRegs`. x86-64 instead recovers
// the return address from a constant rule baked into `cieInitialCFI` and does
// not sample it.
struct EHFrameArch {
  // CIE return_address_register (DWARF number).
  unsigned returnAddrReg;

  // CIE initial instructions: the CFA definition, plus (x86-64 only) the
  // constant rule recovering the return address from the stack.
  std::vector<unsigned char> cieInitialCFI;

  // Callee-saved registers to sample and restore, by DWARF number. On
  // link-register architectures this includes the return-address register.
  std::vector<unsigned> calleeSavedDwarfRegs;

  // This architecture's frame pointer and the DWARF numbers used to express a
  // CFA that is anchored to either the frame or the stack pointer.
  MachRegister framePtr;
  unsigned framePtrDwarf;
  unsigned stackPtrDwarf;

  // DWARF number for a CFA anchored at the frame pointer (if `r` is the frame
  // pointer) or otherwise at the stack pointer.
  unsigned dwarfForFrameOrStack(MachRegister r) const {
    return (r == framePtr) ? framePtrDwarf : stackPtrDwarf;
  }
};

// Returns the parameters for `arch`, or nullptr if synthesizing .eh_frame for
// relocated code is not supported on it (e.g. RISC-V, AMDGPU). Callers must
// skip synthesis when this returns nullptr.
const EHFrameArch* ehFrameArchFor(Architecture arch);

}  // namespace Dyninst

#endif  // DYNINST_EH_FRAME_ARCH_H
