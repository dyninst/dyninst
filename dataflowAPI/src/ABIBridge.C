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

#include "dataflowAPI/src/ABIBridge.h"
#include "abi.h"
#include "registers/registerSet.h"
#include "registers/MachRegister.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"

// This file deliberately does NOT do `using namespace Dyninst`, so that the
// common Dyninst::ABI can be used without clashing with the dataflowAPI ::ABI.

namespace {

  // Convert a base-register set from the common ABI into a bitArray indexed by
  // `idx`. The x86/x86_64 flags register is expanded into the individual flag
  // bits that LivenessAnalyzer::calcRWSets tracks. Registers absent from the
  // index map (e.g. x87 st*, mxcsr) are skipped, matching the previous behavior.
  bitArray toBitArray(Dyninst::abi::registerSet const& regs,
                      Dyninst::DataflowAPI::RegisterMap& idx, bool is64) {
    bitArray b(idx.size());
    auto set = [&](Dyninst::MachRegister r) {
      auto it = idx.find(r);
      if(it != idx.end()) b[it->second] = true;
    };
    for(Dyninst::MachRegister r : regs) {
      if(is64 && r == Dyninst::x86_64::flags) {
        set(Dyninst::x86_64::of); set(Dyninst::x86_64::cf); set(Dyninst::x86_64::pf);
        set(Dyninst::x86_64::af); set(Dyninst::x86_64::zf); set(Dyninst::x86_64::sf);
        set(Dyninst::x86_64::df); set(Dyninst::x86_64::tf); set(Dyninst::x86_64::nt_);
      } else if(!is64 && r == Dyninst::x86::flags) {
        set(Dyninst::x86::of); set(Dyninst::x86::cf); set(Dyninst::x86::pf);
        set(Dyninst::x86::af); set(Dyninst::x86::zf); set(Dyninst::x86::sf);
        set(Dyninst::x86::df); set(Dyninst::x86::tf); set(Dyninst::x86::nt_);
      } else {
        set(r);
      }
    }
    return b;
  }

}

namespace Dyninst { namespace DataflowAPI {

  void buildABIBitArrays(Dyninst::Architecture arch, RegisterMap& idx,
                         bitArray*& returnRegs, bitArray*& callParam,
                         bitArray*& returnRead, bitArray*& callRead,
                         bitArray*& callWritten, bitArray*& syscallRead,
                         bitArray*& syscallWritten, bitArray*& allRegs) {
    Dyninst::ABI newAbi(arch);
    bool is64 = (arch == Dyninst::Arch_x86_64);

    returnRegs     = new bitArray(toBitArray(newAbi.getReturnRegisters(),         idx, is64));
    callParam      = new bitArray(toBitArray(newAbi.getParameterRegisters(),      idx, is64));
    returnRead     = new bitArray(toBitArray(newAbi.getReturnReadRegisters(),     idx, is64));
    callRead       = new bitArray(toBitArray(newAbi.getCallReadRegisters(),       idx, is64));
    callWritten    = new bitArray(toBitArray(newAbi.getCallWrittenRegisters(),    idx, is64));
    syscallRead    = new bitArray(toBitArray(newAbi.getSyscallReadRegisters(),    idx, is64));
    syscallWritten = new bitArray(toBitArray(newAbi.getSyscallWrittenRegisters(), idx, is64));

    allRegs = new bitArray(idx.size());
    allRegs->set();
  }

}}
