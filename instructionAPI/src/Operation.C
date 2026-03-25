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

#include "BinaryFunction.h"
#include "Immediate.h"
#include "Operation_impl.h"
#include "Register.h"
#include "common/src/arch-x86.h"
#include "common/src/vgannotations.h"
#include "concurrent.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <boost/make_shared.hpp>
#include <map>
#include <mutex>

using namespace NS_x86;

namespace Dyninst { namespace InstructionAPI {

  static std::vector<Dyninst::MachRegister> get_read_flags(entryID);
  static std::vector<Dyninst::MachRegister> get_written_flags(entryID);

  RegisterAST::Ptr makeRegFromID(MachRegister regID, unsigned int low, unsigned int high) {
    return boost::make_shared<RegisterAST>(regID, low, high);
  }

  RegisterAST::Ptr makeRegFromID(MachRegister regID) {
    return boost::make_shared<RegisterAST>(regID, 0, regID.size() * 8);
  }

  Operation::Operation(const Operation& o) {
    operationID = o.operationID;
    archDecodedFrom = o.archDecodedFrom;
    prefixID = o.prefixID;
    addrWidth = o.addrWidth;
    segPrefix = o.segPrefix;
    isVectorInsn = o.isVectorInsn;
    mnemonic = o.mnemonic;
  }

  const Operation& Operation::operator=(const Operation& o) {
    operationID = o.operationID;
    archDecodedFrom = o.archDecodedFrom;
    prefixID = o.prefixID;
    addrWidth = o.addrWidth;
    segPrefix = o.segPrefix;
    isVectorInsn = o.isVectorInsn;
    mnemonic = o.mnemonic;
    return *this;
  }

  const Operation::registerSet& Operation::implicitReads() {
    SetUpNonOperandData();

    return otherRead;
  }

  const Operation::registerSet& Operation::implicitWrites() {
    SetUpNonOperandData();

    return otherWritten;
  }

  bool Operation::isRead(Expression::Ptr candidate) {

    SetUpNonOperandData();

    for(RegisterAST::Ptr const& r : otherRead) {
      if(*candidate == *r) {
        return true;
      }
    }
    for(Expression::Ptr const& e : otherEffAddrsRead) {
      if(*candidate == *e) {
        return true;
      }
    }
    return false;
  }

  const Operation::VCSet& Operation::getImplicitMemReads() {
    SetUpNonOperandData();
    return otherEffAddrsRead;
  }

  const Operation::VCSet& Operation::getImplicitMemWrites() {
    SetUpNonOperandData();
    return otherEffAddrsWritten;
  }

  bool Operation::isWritten(Expression::Ptr candidate) {

    SetUpNonOperandData();

    for(RegisterAST::Ptr const& r : otherWritten) {
      if(*candidate == *r) {
        return true;
      }
    }
    for(Expression::Ptr const& e : otherEffAddrsWritten) {
      if(*candidate == *e) {
        return true;
      }
    }
    return false;
  }

  std::string Operation::format() const {
    if(mnemonic != "") {
      return mnemonic;
    }
    dyn_hash_map<prefixEntryID, std::string>::const_iterator foundPrefix =
        prefixEntryNames_IAPI.find(prefixID);
    dyn_hash_map<entryID, std::string>::const_iterator found = entryNames_IAPI.find(operationID);
    std::string result;
    if(foundPrefix != prefixEntryNames_IAPI.end()) {
      result += (foundPrefix->second + " ");
    }
    if(found != entryNames_IAPI.end()) {
      result += found->second;
    } else {
      result += "[INVALID]";
    }
    return result;
  }

  entryID Operation::getID() const { return operationID; }

  prefixEntryID Operation::getPrefixID() const { return prefixID; }

  struct OperationMaps {
    typedef dyn_c_hash_map<entryID, Operation::registerSet> reg_info_t;
    typedef dyn_c_hash_map<entryID, Operation::VCSet> mem_info_t;

  public:
    OperationMaps(Architecture arch) {
      thePC.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(arch))));
      pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(arch))));
      pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
      stackPointer.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
      stackPointerAsExpr.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
      framePointer.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(arch))));
      spAndBP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
      spAndBP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(arch))));
      si.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rsi : x86::esi)));
      di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rdi : x86::edi)));
      si_and_di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rsi : x86::esi)));
      si_and_di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rdi : x86::edi)));

      nonOperandRegisterReads.insert(make_pair(e_call, pcAndSP));
      nonOperandRegisterReads.insert(make_pair(e_ret_near, stackPointer));
      nonOperandRegisterReads.insert(make_pair(e_ret_far, stackPointer));
      nonOperandRegisterReads.insert(make_pair(e_leave, framePointer));
      nonOperandRegisterReads.insert(make_pair(e_enter, spAndBP));

      nonOperandRegisterWrites.insert(make_pair(e_call, pcAndSP));
      nonOperandRegisterWrites.insert(make_pair(e_ret_near, pcAndSP));
      nonOperandRegisterWrites.insert(make_pair(e_ret_far, pcAndSP));
      nonOperandRegisterWrites.insert(make_pair(e_leave, spAndBP));
      nonOperandRegisterWrites.insert(make_pair(e_enter, spAndBP));

      nonOperandRegisterWrites.insert(make_pair(e_loop, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_loope, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_loopne, thePC));

      nonOperandRegisterWrites.insert(make_pair(e_jb, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jb_jnaej_j, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jbe, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jcxz_jec, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jl, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jle, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jmp, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jae, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jnb_jae_j, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_ja, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jge, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jg, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jno, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jnp, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jns, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jne, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jo, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_jp, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_js, thePC));
      nonOperandRegisterWrites.insert(make_pair(e_je, thePC));

      nonOperandMemoryReads.insert(make_pair(e_pop, stackPointerAsExpr));
      nonOperandMemoryReads.insert(make_pair(e_popal, stackPointerAsExpr));
      nonOperandMemoryReads.insert(make_pair(e_popaw, stackPointerAsExpr));
      nonOperandMemoryWrites.insert(make_pair(e_push, stackPointerAsExpr));
      nonOperandMemoryWrites.insert(make_pair(e_pushal, stackPointerAsExpr));
      nonOperandMemoryWrites.insert(make_pair(e_call, stackPointerAsExpr));
      nonOperandMemoryReads.insert(make_pair(e_ret_near, stackPointerAsExpr));
      nonOperandMemoryReads.insert(make_pair(e_ret_far, stackPointerAsExpr));
      nonOperandMemoryReads.insert(make_pair(e_leave, stackPointerAsExpr));

      nonOperandRegisterWrites.insert(make_pair(e_cmpsb, si_and_di));
      nonOperandRegisterWrites.insert(make_pair(e_cmpsd, si_and_di));
      nonOperandRegisterWrites.insert(make_pair(e_cmpsw, si_and_di));
      nonOperandRegisterWrites.insert(make_pair(e_movsb, si_and_di));
      nonOperandRegisterWrites.insert(make_pair(e_movsd, si_and_di));
      nonOperandRegisterWrites.insert(make_pair(e_movsw, si_and_di));
      nonOperandRegisterWrites.insert(make_pair(e_insb, di));
      nonOperandRegisterWrites.insert(make_pair(e_insd, di));
      nonOperandRegisterWrites.insert(make_pair(e_insw, di));
      nonOperandRegisterWrites.insert(make_pair(e_stosb, di));
      nonOperandRegisterWrites.insert(make_pair(e_stosd, di));
      nonOperandRegisterWrites.insert(make_pair(e_stosw, di));
      nonOperandRegisterWrites.insert(make_pair(e_scasb, di));
      nonOperandRegisterWrites.insert(make_pair(e_scasd, di));
      nonOperandRegisterWrites.insert(make_pair(e_scasw, di));
      nonOperandRegisterWrites.insert(make_pair(e_lodsb, di));
      nonOperandRegisterWrites.insert(make_pair(e_lodsd, di));
      nonOperandRegisterWrites.insert(make_pair(e_lodsw, di));
      nonOperandRegisterWrites.insert(make_pair(e_outsb, di));
      nonOperandRegisterWrites.insert(make_pair(e_outsd, di));
      nonOperandRegisterWrites.insert(make_pair(e_outsw, di));
    }

    Operation::registerSet thePC;
    Operation::registerSet pcAndSP;
    Operation::registerSet stackPointer;
    Operation::VCSet stackPointerAsExpr;
    Operation::registerSet framePointer;
    Operation::registerSet spAndBP;
    Operation::registerSet si;
    Operation::registerSet di;
    Operation::registerSet si_and_di;

    reg_info_t nonOperandRegisterReads;
    reg_info_t nonOperandRegisterWrites;

    mem_info_t nonOperandMemoryReads;
    mem_info_t nonOperandMemoryWrites;
  };

  OperationMaps op_data_32(Arch_x86);
  OperationMaps op_data_64(Arch_x86_64);

  const OperationMaps& op_data(Architecture arch) {
    switch(arch) {
      case Arch_x86: return op_data_32;
      case Arch_x86_64: return op_data_64;
      default: return op_data_32;
    }
  }

  void Operation::SetUpNonOperandData() {
    if(archDecodedFrom != Arch_x86 && archDecodedFrom != Arch_x86_64)
      return;
    std::call_once(data_initialized, [&]() {
      if(prefixID == prefix_rep || prefixID == prefix_repnz) {
        otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
        otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ecx : x86_64::rcx));
        otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ecx : x86_64::rcx));
        if(prefixID == prefix_repnz) {
          otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
        }
      }
      switch(segPrefix) {
        case PREFIX_SEGCS:
          otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cs : x86_64::cs));
          break;
        case PREFIX_SEGDS:
          otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ds : x86_64::ds));
          break;
        case PREFIX_SEGES:
          otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::es : x86_64::es));
          break;
        case PREFIX_SEGFS:
          otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::fs : x86_64::fs));
          break;
        case PREFIX_SEGGS:
          otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::gs : x86_64::gs));
          break;
        case PREFIX_SEGSS:
          otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ss : x86_64::ss));
          break;
      }

      OperationMaps::reg_info_t::const_accessor a, b;
      if(op_data(archDecodedFrom).nonOperandRegisterReads.find(a, operationID)) {
        otherRead.insert(a->second.begin(), a->second.end());
      }
      if(op_data(archDecodedFrom).nonOperandRegisterWrites.find(b, operationID)) {
        otherWritten.insert(b->second.begin(), b->second.end());
      }
      OperationMaps::mem_info_t::const_accessor c, d;
      if(op_data(archDecodedFrom).nonOperandMemoryReads.find(c, operationID)) {
        otherEffAddrsRead.insert(c->second.begin(), c->second.end());
      }
      if(operationID == e_push) {
        BinaryFunction::funcT::Ptr adder(new BinaryFunction::addResult());
        // special case for push: we write at the new value of the SP.

        if(addrWidth == Result_Type{}) {
          addrWidth = (archDecodedFrom == Arch_x86) ? u32 : u64;
        }

        Result dummy(addrWidth, 0);
        Expression::Ptr push_addr(new BinaryFunction(
            *(op_data(archDecodedFrom).stackPointerAsExpr.begin()),
            Immediate::makeImmediate(Result(s8, -(dummy.size()))), addrWidth, adder));

        otherEffAddrsWritten.insert(push_addr);

      } else {
        if(op_data(archDecodedFrom).nonOperandMemoryWrites.find(d, operationID)) {
          otherEffAddrsWritten.insert(d->second.begin(), d->second.end());
        }
      }

      for(MachRegister m : get_read_flags(operationID)) {
        switch(m) {
          case x86::icf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cf : x86_64::cf));
            break;
          case x86::ipf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::pf : x86_64::pf));
            break;
          case x86::iaf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::af : x86_64::af));
            break;
          case x86::izf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
            break;
          case x86::isf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::sf : x86_64::sf));
            break;
          case x86::itf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::tf : x86_64::tf));
            break;
          case x86::idf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
            break;
          case x86::iof:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::of : x86_64::of));
            break;
          case x86::int_:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::nt_ : x86_64::nt_));
            break;
          case x86::iif_:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::if_ : x86_64::if_));
            break;
          case x86::irf:
            otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::rf : x86_64::rf));
            break;
          default:
            break;
        }
      }

      for(MachRegister m : get_written_flags(operationID)) {
        switch(m) {
          case x86::icf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cf : x86_64::cf));
            break;
          case x86::ipf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::pf : x86_64::pf));
            break;
          case x86::iaf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::af : x86_64::af));
            break;
          case x86::izf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
            break;
          case x86::isf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::sf : x86_64::sf));
            break;
          case x86::itf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::tf : x86_64::tf));
            break;
          case x86::idf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
            break;
          case x86::iof:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::of : x86_64::of));
            break;
          case x86::int_:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::nt_ : x86_64::nt_));
            break;
          case x86::iif_:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::if_ : x86_64::if_));
            break;
          case x86::irf:
            otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::rf : x86_64::rf));
            break;
          default:
            break;
        }
      }
    });
    return;
  }

  bool Operation::operator==(Operation const &rhs) const {
    return operationID == rhs.operationID &&
           archDecodedFrom == rhs.archDecodedFrom &&
           mnemonic == rhs.mnemonic;
  }

  std::vector<Dyninst::MachRegister> get_read_flags(entryID id) {
    switch(id) {
      case e_aaa: return {x86::af};
      case e_aas: return {x86::af};
      case e_adc: return {x86::cf};
      case e_cmovbe: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmove: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovb: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovae: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmova: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovne: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovle: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovl: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovge: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovno: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovns: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovo: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovp: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovnp: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_cmovs: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_daa: return {x86::af,x86::cf};
      case e_das: return {x86::af,x86::cf};
      case e_div: return {x86::af,x86::cf};
      case e_insb: return {x86::df};
      case e_insw: return {x86::df};
      case e_insd: return {x86::df};
      case e_into: return {x86::of};
      case e_iret: return {x86::nt_};
      case e_jb: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jb_jnaej_j: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jbe: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jl: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jle: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jae: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jnb_jae_j: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_ja: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jge: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jg: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jno: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jnp: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jns: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jne: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jo: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_jp: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_js: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_je: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_lodsb: return {x86::df};
      case e_lodsd: return {x86::df};
      case e_lodsw: return {x86::df};
      case e_loope: return {x86::zf};
      case e_loopne: return {x86::zf};
      case e_outsb: return {x86::df};
      case e_outsw: return {x86::df};
      case e_outsd: return {x86::df};
      case e_rcl: return {x86::cf};
      case e_rcr: return {x86::cf};
      case e_rol: return {x86::cf};
      case e_ror: return {x86::cf};
      case e_sahf: return {x86::sf,x86::zf,x86::af,x86::pf,x86::cf};
      case e_salc: return {x86::cf};
      case e_sbb: return {x86::cf};
      case e_setb: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setbe: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setl: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setle: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setae: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_seta: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setge: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setg: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setno: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setnp: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setns: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setne: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_seto: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_setp: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_sets: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_sete: return {x86::of,x86::sf,x86::zf,x86::pf,x86::cf};
      case e_stosb: return {x86::df};
      case e_stosd: return {x86::df};
      case e_stosw: return {x86::df};
      case e_scasb: return {x86::df};
      case e_scasw: return {x86::df};
      case e_scasd: return {x86::df};
      case e_popcnt: return {x86::of,x86::sf,x86::zf,x86::af,x86::cf,x86::pf};
      default:
        return {};
    }
  }

  std::vector<MachRegister> get_written_flags(entryID id) {
    auto standardFlags = {x86::of,x86::sf,x86::zf,x86::af,x86::pf,x86::cf};
    switch(id) {
      case e_aaa: return standardFlags;
      case e_aad: return standardFlags;
      case e_aam: return standardFlags;
      case e_aas: return standardFlags;
      case e_adc: return standardFlags;
      case e_add: return standardFlags;
      case e_and: return standardFlags;
      case e_arpl: return {x86::zf};
      case e_bsf: return standardFlags;
      case e_bsr: return standardFlags;
      case e_bt: return standardFlags;
      case e_bts: return standardFlags;
      case e_btr: return standardFlags;
      case e_btc: return standardFlags;
      case e_clc: return {x86::cf};
      case e_cld: return {x86::df};
      case e_cli: return {x86::if_};
      case e_cmc: return {x86::cf};
      case e_cmp: return standardFlags;
      case e_cmpsb: return standardFlags;
      case e_cmpsd: return standardFlags;
      case e_cmpss: return standardFlags;
      case e_cmpsw: return standardFlags;
      case e_cmpxchg: return standardFlags;
      case e_cmpxchg8b: return {x86::zf};
      case e_comisd: return standardFlags;
      case e_comiss: return standardFlags;
      case e_daa: return standardFlags;
      case e_das: return standardFlags;
      case e_dec: return {x86::of,x86::sf,x86::zf,x86::af,x86::pf};
      case e_div: return standardFlags;
      case e_idiv: return standardFlags;
      case e_imul: return standardFlags;
      case e_inc: return {x86::of,x86::sf,x86::zf,x86::af,x86::pf};
      case e_int: return {x86::tf,x86::nt_};
      case e_int3: return {x86::tf,x86::nt_};
      case e_int80: return {x86::tf,x86::nt_};
      case e_into: return {x86::tf,x86::nt_};
      case e_ucomisd: return standardFlags;
      case e_ucomiss: return standardFlags;
      case e_iret: return {x86::of,x86::sf,x86::zf,x86::af,x86::pf,x86::cf,x86::tf,x86::if_,x86::df};
      case e_lar: return {x86::zf};
      case e_lsl: return {x86::zf};
      case e_mul: return standardFlags;
      case e_neg: return standardFlags;
      case e_or: return standardFlags;
      case e_popf: return {x86::of,x86::sf,x86::zf,x86::af,x86::pf,x86::cf,x86::tf,x86::if_,x86::df,x86::nt_};
      case e_popfd: return {x86::of,x86::sf,x86::zf,x86::af,x86::pf,x86::cf,x86::tf,x86::if_,x86::df,x86::nt_};
      case e_rcl: return {x86::of,x86::cf};
      case e_rcr: return {x86::of,x86::cf};
      case e_rol: return {x86::of,x86::cf};
      case e_ror: return {x86::of,x86::cf};
      case e_rsm: return {x86::of,x86::sf,x86::zf,x86::af,x86::pf,x86::cf,x86::tf,x86::if_,x86::df,x86::nt_,x86::rf};
      case e_sar: return standardFlags;
      case e_shr: return standardFlags;
      case e_sbb: return standardFlags;
      case e_shld: return standardFlags;
      case e_shrd: return standardFlags;
      case e_shl: return standardFlags;
      case e_stc: return {x86::cf};
      case e_std: return {x86::df};
      case e_sti: return {x86::if_};
      case e_sub: return standardFlags;
      case e_test: return standardFlags;
      case e_verr: return {x86::zf};
      case e_verw: return {x86::zf};
      case e_xadd: return standardFlags;
      case e_xor: return standardFlags;
      case e_scasb: return standardFlags;
      case e_scasw: return standardFlags;
      case e_scasd: return standardFlags;
      case e_pcmpestri: return standardFlags;
      case e_pcmpestrm: return standardFlags;
      case e_pcmpistri: return standardFlags;
      case e_pcmpistrm: return standardFlags;
      case e_ptest: return standardFlags;
      default:
        return {};
    }
  }

}}
