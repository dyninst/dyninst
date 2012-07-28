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
#include "SymEvalPolicy.h"

#include "dyn_regs.h"

using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace Dyninst::InstructionAPI;

SymEvalPolicy::SymEvalPolicy(Result_t &r,
			     Address a,
			     Dyninst::Architecture ac,
                             Instruction::Ptr insn) :
  res(r),
  arch(ac),
  addr(a),
  ip_(Handle<32>(wrap(Absloc::makePC(arch)))),
  failedTranslate_(false),
  insn_(insn) {

  // We also need to build aaMap FTW!!!
  for (Result_t::iterator iter = r.begin();
       iter != r.end(); ++iter) {
    Assignment::Ptr a = iter->first;
    // For a different instruction...
    if (a->addr() != addr) continue; 
    AbsRegion &o = a->out();

    if (o.containsOfType(Absloc::Register)) {
      // We're assuming this is a single register...
      //std::cerr << "Marking register " << a << std::endl;
      aaMap[o.absloc()] = a;
    }
    else {
      // Use sufficiently-unique (Heap,0) Absloc
      // to represent a definition to a memory absloc
      aaMap[Absloc(0)] = a;
    }
  }
}
  
void SymEvalPolicy::undefinedInstruction(SgAsmx86Instruction *) {
   undefinedInstructionCommon();
}

void SymEvalPolicy::undefinedInstruction(SgAsmPowerpcInstruction *) {
   // Log insn details here

   undefinedInstructionCommon();
}

void SymEvalPolicy::undefinedInstructionCommon() {
   for (std::map<Absloc, Assignment::Ptr>::iterator iter = aaMap.begin();
        iter != aaMap.end(); ++iter) {
      res[iter->second] = getBottomAST();
   }
   failedTranslate_ = true;
}

void SymEvalPolicy::startInstruction(SgAsmx86Instruction *) {
}

void SymEvalPolicy::finishInstruction(SgAsmx86Instruction *) {
}
void SymEvalPolicy::startInstruction(SgAsmPowerpcInstruction *) {
}

void SymEvalPolicy::finishInstruction(SgAsmPowerpcInstruction *) {
}

Absloc SymEvalPolicy::convert(PowerpcRegisterClass regtype, int regNum)
{
    switch(regtype)
    {
        case powerpc_regclass_gpr:
            return Absloc(MachRegister(ppc32::GPR | regNum | Arch_ppc32));
        case powerpc_regclass_spr:
            return Absloc(MachRegister(ppc32::SPR | regNum | Arch_ppc32));
        case powerpc_regclass_cr:
        {
            if(regNum == -1)
                return Absloc(ppc32::cr);
            switch(regNum)
            {
                case 0:
                    return Absloc(ppc32::cr0);
                case 1:
                    return Absloc(ppc32::cr1);
                case 2:
                    return Absloc(ppc32::cr2);
                case 3:
                    return Absloc(ppc32::cr3);
                case 4:
                    return Absloc(ppc32::cr4);
                case 5:
                    return Absloc(ppc32::cr5);
                case 6:
                    return Absloc(ppc32::cr6);
                case 7:
                    return Absloc(ppc32::cr7);
                default:
                    assert(!"bad CR field!");
                    return Absloc();
            }
        }
        default:
            assert(!"unknown power register class!");
            return Absloc();
            
    }
}
        
Absloc SymEvalPolicy::convert(X86GeneralPurposeRegister r)
{

  MachRegister mreg;
  switch (r) {
    case x86_gpr_ax:
      mreg = x86::eax;
      break;
    case x86_gpr_cx:
      mreg = x86::ecx;
      break;
    case x86_gpr_dx:
      mreg = x86::edx;
      break;
    case x86_gpr_bx:
      mreg = x86::ebx;
      break;
    case x86_gpr_sp:
      mreg = x86::esp;
      break;
    case x86_gpr_bp:
      mreg = x86::ebp;
      break;
    case x86_gpr_si:
      mreg = x86::esi;
      break;
    case x86_gpr_di:
      mreg = x86::edi;
      break;
    default:
      break;
  }

  return Absloc(mreg);;
}

Absloc SymEvalPolicy::convert(X86SegmentRegister r)
{
  MachRegister mreg;
  switch (r) {
    case x86_segreg_es:
      mreg = x86::es;
      break;
    case x86_segreg_cs:
      mreg = x86::cs;
      break;
    case x86_segreg_ss:
      mreg = x86::ss;
      break;
    case x86_segreg_ds:
      mreg = x86::ds;
      break;
    case x86_segreg_fs:
      mreg = x86::fs;
      break;
    case x86_segreg_gs:
      mreg = x86::gs;
      break;
    default:
      break;
  }

  return Absloc(mreg);
}

Absloc SymEvalPolicy::convert(X86Flag f)
{
   switch (f) {
      case x86_flag_cf:
         return Absloc(x86::cf);
      case x86_flag_1:
         return Absloc(x86::flag1);
      case x86_flag_pf:
         return Absloc(x86::pf);
      case x86_flag_3:
         return Absloc(x86::flag3);
      case x86_flag_af:
         return Absloc(x86::af);
      case x86_flag_5:
         return Absloc(x86::flag5);
      case x86_flag_zf:
         return Absloc(x86::zf);
      case x86_flag_sf:
         return Absloc(x86::sf);
      case x86_flag_tf:
         return Absloc(x86::tf);
      case x86_flag_if:
         return Absloc(x86::if_);
      case x86_flag_df:
         return Absloc(x86::df);
      case x86_flag_of:
         return Absloc(x86::of);
      case x86_flag_iopl0:
         return Absloc(x86::flagc);
      case x86_flag_iopl1:
         return Absloc(x86::flagd);
      case x86_flag_nt:
         return Absloc(x86::nt_);
      case x86_flag_15:
         return Absloc(x86::flagf);
      default:
         assert(0);
         return Absloc();
  }
}

std::ostream &operator<<(std::ostream &os, const ROSEOperation &o) {
  os << o.format();
  return os;
}

std::ostream &operator<<(std::ostream &os, const Constant &o) {
  os << o.format();
  return os;
}


std::ostream &operator<<(std::ostream &os, const Variable &v) {
  os << v.format();
  return os;
}

