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
#include "registers/abi.h"
#include "registers/abstract_regs.h"

#include <cassert>
#include <cstdint>
#include <set>

namespace Dyninst {

  namespace {

    // The definition used for an architecture whose ABI is not modelled:
    // every register may be passed or returned and none is preserved. So every
    // register is live at a call, a return and a system call, and a call or
    // system call may clobber every computational register.
    abi::definition unmodelled(Architecture arch) {
      abi::definition d;
      for(auto r : MachRegister::getAllRegistersForArch(arch)) {
        d.function.params.insert(r);
      }
      d.function.returns = d.function.params;
      d.syscall.params = d.function.params;
      d.syscall.returns = d.function.params;
      return d;
    }

    abi::definition definitionFor(Architecture arch) {
      switch(arch) {
        case Arch_x86: return abi::make_x86();
        case Arch_x86_64: return abi::make_x86_64();
        case Arch_ppc64: return abi::make_ppc64();

        // Architectures whose ABI is not modelled.
        case Arch_none:
        case Arch_ppc32:
        case Arch_aarch32:
        case Arch_aarch64:
        case Arch_riscv64:
        case Arch_cuda:
        case Arch_amdgpu_gfx908:
        case Arch_amdgpu_gfx90a:
        case Arch_amdgpu_gfx940:
        case Arch_amdgpu_gfx950:
        case Arch_intelGen9:
          break;
      }
      return unmodelled(arch);
    }

    bool isComputational(MachRegister r) {
      return r.isGeneralPurpose() || r.isVector() || r.isFloatingPoint() || r.isFlag();
    }

  }

  template <Architecture A>
  ABI const& ABI::instance() {
    static ABI const abi(A, definitionFor(A));
    return abi;
  }

  ABI const& ABI::get(Architecture arch) {
    switch(arch) {
      case Arch_none: return instance<Arch_none>();
      case Arch_x86: return instance<Arch_x86>();
      case Arch_x86_64: return instance<Arch_x86_64>();
      case Arch_ppc32: return instance<Arch_ppc32>();
      case Arch_ppc64: return instance<Arch_ppc64>();
      case Arch_aarch32: return instance<Arch_aarch32>();
      case Arch_aarch64: return instance<Arch_aarch64>();
      case Arch_riscv64: return instance<Arch_riscv64>();
      case Arch_cuda: return instance<Arch_cuda>();
      case Arch_amdgpu_gfx908: return instance<Arch_amdgpu_gfx908>();
      case Arch_amdgpu_gfx90a: return instance<Arch_amdgpu_gfx90a>();
      case Arch_amdgpu_gfx940: return instance<Arch_amdgpu_gfx940>();
      case Arch_amdgpu_gfx950: return instance<Arch_amdgpu_gfx950>();
      case Arch_intelGen9: return instance<Arch_intelGen9>();
    }
    assert(!"Unknown architecture");
    return instance<Arch_none>();
  }

  ABI::ABI(Architecture arch, abi::definition const& def)
      : arch_(arch),
        namespace_(def.register_namespace == Arch_none ? arch : def.register_namespace) {
    std::set<MachRegister> bases;
    for(auto r : MachRegister::getAllRegistersForArch(namespace_)) {
      auto const base = r.getBaseRegister();
      if(base.isValid()) {
        bases.insert(base);
      }
    }
    layout_.assign(bases.begin(), bases.end());
    for(std::size_t i = 0; i < layout_.size(); i++) {
      index_.emplace(layout_[i].val(), static_cast<int>(i));
    }

    all_ = makeEmpty();
    all_.insertAll();

    LivenessSet computational = makeEmpty();
    for(auto r : layout_) {
      if(isComputational(r)) {
        computational.insert(r);
      }
    }

    params_ = LivenessSet(*this, def.function.params);
    returns_ = LivenessSet(*this, def.function.returns);
    calleeSaved_ = LivenessSet(*this, def.function.preserved);
    LivenessSet const globals(*this, def.function.globals);

    callRead_ = params_ | globals;
    callWritten_ = computational - calleeSaved_;
    returnRead_ = returns_ | calleeSaved_ | globals;

    syscallRead_ = LivenessSet(*this, def.syscall.params) | globals;
    syscallWritten_ = computational - LivenessSet(*this, def.syscall.preserved);
  }

  MachRegister ABI::canonicalize(MachRegister reg) const {
    auto const ns = reg.getArchitecture();
    if(ns == arch_ && ns != namespace_) {
      // The two namespaces encode the same register with the same low bits.
      auto const regBits = static_cast<uint32_t>(reg.val()) & ~static_cast<uint32_t>(ns);
      reg = MachRegister(static_cast<int32_t>(regBits | static_cast<uint32_t>(namespace_)));
    } else if(ns != namespace_) {
      return InvalidReg;
    }
    return reg.getBaseRegister();
  }

  int ABI::getIndex(MachRegister reg) const {
    auto const it = index_.find(canonicalize(reg).val());
    return (it == index_.end()) ? -1 : it->second;
  }

}
