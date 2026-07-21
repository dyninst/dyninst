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

#include "abi.h"
#include "ABI/x86.h"
#include "ABI/x86_64.h"
#include "ABI/ppc64.h"
#include "ABI/aarch64.h"
#include "registers/registerSet.h"
#include "ABI/architecture.h"

namespace Dyninst { namespace abi {

  struct abi_impl final {
    Dyninst::Architecture arch{};
    Dyninst::abi::architecture machine;

    // Registers of the architecture, and the semantic sets derived from the
    // per-architecture interface_definitions.
    registerSet allRegs;
    registerSet params;
    registerSet returnRegs;
    registerSet returnRead;
    registerSet callRead;
    registerSet callWritten;
    registerSet syscallRead;
    registerSet syscallWritten;

    abi_impl(Dyninst::Architecture a) : arch(a) {
      switch(a) {
        case Dyninst::Arch_x86:
          machine = abi::make_x86();
          break;
        case Dyninst::Arch_x86_64:
          machine = abi::make_x86_64();
          break;
        case Dyninst::Arch_ppc64:
          machine = abi::make_ppc64();
          break;
        case Dyninst::Arch_aarch64:
          machine = abi::make_aarch64();
          break;
        case Dyninst::Arch_ppc32:
        case Dyninst::Arch_aarch32:
        case Dyninst::Arch_amdgpu_gfx908:
        case Dyninst::Arch_amdgpu_gfx90a:
        case Dyninst::Arch_amdgpu_gfx940:
        case Dyninst::Arch_cuda:
        case Dyninst::Arch_intelGen9:
        case Dyninst::Arch_none:
          break;
      }
      derive();
    }

    // Map the four-set ABI model onto the semantic sets that liveness analysis
    // consumes. Unsupported architectures leave every set empty.
    //
    // Every set is expressed in terms of *base* registers (e.g. rbx rather than
    // ebx/bx/bl). This is what makes the "written" sets correct: without it,
    // subtracting a preserved register (rbx) from the full register list would
    // leave its sub-registers (ebx, ...) behind and wrongly mark them clobbered.
    // Consumers must likewise canonicalize a register to its base before
    // querying these sets (MachRegister::getBaseRegister()).
    void derive() {
      auto baseSet = [](registerSet const& s) {
        registerSet out;
        for(auto const& r : s) {
          out.insert(r.getBaseRegister());
        }
        return out;
      };

      for(auto const& r : MachRegister::getAllRegistersForArch(arch)) {
        allRegs.insert(r.getBaseRegister());
      }

      auto const params_    = baseSet(machine.function.params);
      auto const returns_   = baseSet(machine.function.returns);
      auto const preserved_ = baseSet(machine.function.preserved);
      auto const globals_   = baseSet(machine.function.globals);
      auto const scParams_    = baseSet(machine.syscall.params);
      auto const scPreserved_ = baseSet(machine.syscall.preserved);

      params      = params_;
      returnRegs  = returns_;

      // Live at a return: return values plus everything the callee must have
      // preserved for the caller (callee-saved registers and ABI globals).
      returnRead  = returns_ | preserved_ | globals_;

      // A call may read its argument registers and the ABI globals.
      callRead    = params_ | globals_;

      // A call may clobber anything that is not callee-saved or an ABI global.
      callWritten = allRegs - (preserved_ | globals_);

      syscallRead    = scParams_ | globals_;
      syscallWritten = allRegs - (scPreserved_ | globals_);
    }
  };

}}

Dyninst::ABI::ABI(Dyninst::Architecture a) : impl(new Dyninst::abi::abi_impl(a)) {}
Dyninst::ABI::~ABI() = default;
Dyninst::ABI::ABI(ABI&&) noexcept = default;
Dyninst::ABI& Dyninst::ABI::operator=(ABI&&) noexcept = default;

int Dyninst::ABI::addressWidth() const { return impl->machine.address_width; }

Dyninst::abi::registerSet const& Dyninst::ABI::getParameterRegisters() const {
  return impl->params;
}
Dyninst::abi::registerSet const& Dyninst::ABI::getReturnRegisters() const {
  return impl->returnRegs;
}
Dyninst::abi::registerSet const& Dyninst::ABI::getReturnReadRegisters() const {
  return impl->returnRead;
}
Dyninst::abi::registerSet const& Dyninst::ABI::getCallReadRegisters() const {
  return impl->callRead;
}
Dyninst::abi::registerSet const& Dyninst::ABI::getCallWrittenRegisters() const {
  return impl->callWritten;
}
Dyninst::abi::registerSet const& Dyninst::ABI::getSyscallReadRegisters() const {
  return impl->syscallRead;
}
Dyninst::abi::registerSet const& Dyninst::ABI::getSyscallWrittenRegisters() const {
  return impl->syscallWritten;
}
