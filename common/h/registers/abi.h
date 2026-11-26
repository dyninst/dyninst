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

#ifndef DYNINST_COMMON_REGISTERS_ABI_H
#define DYNINST_COMMON_REGISTERS_ABI_H

#include "Architecture.h"
#include "dyninst_visibility.h"
#include "registers/LivenessSet.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Dyninst {

  namespace abi {
    struct definition;
  }

  /*
   * The application binary interface of one architecture.
   *
   * An ABI fixes two things:
   *
   *  - The liveness layout: the registers tracked by a LivenessSet and the bit
   *    each one occupies. There is one bit per base register
   *    (MachRegister::getBaseRegister()), so every alias of a physical register
   *    (eax/ax/al, xmm0/ymm0, w0, ...) shares the bit of its base register.
   *
   *  - The calling-convention register sets, each available as a LivenessSet
   *    so it can be combined directly with liveness results. Use
   *    LivenessSet::registers() for the equivalent register_set.
   *
   * ABIs are immutable singletons; get() is safe to call from any thread.
   */
  class DYNINST_EXPORT ABI final {
  public:
    static ABI const& get(Architecture arch);

    ABI(ABI const&) = delete;
    ABI& operator=(ABI const&) = delete;

    Architecture getArchitecture() const { return arch_; }
    unsigned getAddressWidth() const { return getArchAddressWidth(arch_); }

    // Number of registers tracked, i.e. the width of every LivenessSet.
    std::size_t size() const { return layout_.size(); }

    // The bit for `reg`, or -1 if this ABI does not track it.
    int getIndex(MachRegister reg) const;

    // The register tracked at `index`; its inverse is getIndex.
    MachRegister getRegister(std::size_t index) const { return layout_[index]; }

    // Every tracked register, in bit order.
    std::vector<MachRegister> const& getRegisters() const { return layout_; }

    // The canonical form of `reg` in this ABI's layout.
    MachRegister canonicalize(MachRegister reg) const;

    // All registers, and an empty set.
    LivenessSet const& getAllRegs() const { return all_; }
    LivenessSet makeEmpty() const { return LivenessSet(*this); }

    // Function calling sequence.
    LivenessSet const& getParameterRegisters() const { return params_; }
    LivenessSet const& getReturnRegisters() const { return returns_; }
    LivenessSet const& getCalleeSavedRegisters() const { return calleeSaved_; }

    // Registers a call may read: the arguments and the ABI globals.
    LivenessSet const& getCallReadRegisters() const { return callRead_; }

    // Registers a call may write: the caller-saved registers, i.e. the
    // general-purpose, vector, floating-point and flag registers that are not
    // callee-saved. "Flag" is MachRegister::isFlag(), which also covers, e.g.,
    // the aarch64 daif and pstate and the riscv64 CSRs. Other control,
    // segment and system registers are never treated as clobbered by a call.
    LivenessSet const& getCallWrittenRegisters() const { return callWritten_; }

    // Registers live at a return: return values, callee-saved registers and
    // the ABI globals.
    LivenessSet const& getReturnReadRegisters() const { return returnRead_; }

    // System calling sequence.
    LivenessSet const& getSyscallReadRegisters() const { return syscallRead_; }
    LivenessSet const& getSyscallWrittenRegisters() const { return syscallWritten_; }

  private:
    ABI(Architecture arch, abi::definition const& def);

    // The singleton of architecture A.
    template <Architecture A>
    static ABI const& instance();

    Architecture arch_;
    Architecture namespace_;
    std::vector<MachRegister> layout_;
    std::unordered_map<int32_t, int> index_;

    LivenessSet all_;
    LivenessSet params_;
    LivenessSet returns_;
    LivenessSet calleeSaved_;
    LivenessSet callRead_;
    LivenessSet callWritten_;
    LivenessSet returnRead_;
    LivenessSet syscallRead_;
    LivenessSet syscallWritten_;
  };

}

#endif
