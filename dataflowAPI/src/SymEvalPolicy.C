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

#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"


using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace Dyninst::InstructionAPI;

SymEvalPolicy::SymEvalPolicy(Result_t &r,
                             Address a,
                             Dyninst::Architecture ac,
                             Instruction insn) :
  res(r),
  arch(ac),
  addr(a),
  ip_(Handle<32>(wrap(Absloc::makePC(arch)))),
  failedTranslate_(false),
  insn_(insn) {

  // We also need to build aaMap FTW!!!
  for (Result_t::iterator iter = r.begin();
       iter != r.end(); ++iter) {
    Assignment::Ptr ap = iter->first;
    // For a different instruction...
    if (ap->addr() != addr) continue; 
    AbsRegion &o = ap->out();

    if (o.containsOfType(Absloc::Register)) {
      // We're assuming this is a single register...
      //std::cerr << "Marking register " << ap << std::endl;
      aaMap[o.absloc()] = ap;
    }
    else {
      // Use sufficiently-unique (Heap,0) Absloc
      // to represent a definition to a memory absloc
      aaMap[Absloc(0)] = ap;
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
            return Absloc(MachRegister(ppc32::GPR | regNum | arch));
        case powerpc_regclass_spr:
            return Absloc(MachRegister(ppc32::SPR | regNum | arch));
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

       
SymEvalPolicy_64::SymEvalPolicy_64(Result_t &r,
                                   Address a,
				   Dyninst::Architecture ac,
				   Instruction insn) :
  res(r),
  arch(ac),
  addr(a),
  ip_(Handle<64>(wrap(Absloc::makePC(arch)))),
  failedTranslate_(false),
  insn_(insn) {

  // We also need to build aaMap FTW!!!
  for (Result_t::iterator iter = r.begin();
       iter != r.end(); ++iter) {
    Assignment::Ptr ap = iter->first;
    // For a different instruction...
    if (ap->addr() != addr) continue; 
    AbsRegion &o = ap->out();

    if (o.containsOfType(Absloc::Register)) {
      // We're assuming this is a single register...
      //std::cerr << "Marking register " << a << std::endl;
      aaMap[o.absloc()] = ap;
    }
    else {
      // Use sufficiently-unique (Heap,0) Absloc
      // to represent a definition to a memory absloc
      aaMap[Absloc(0)] = ap;
    }
  }
}
  
void SymEvalPolicy_64::undefinedInstruction(SgAsmx86Instruction *) {
   undefinedInstructionCommon();
}

void SymEvalPolicy_64::undefinedInstruction(SgAsmPowerpcInstruction *) {
   // Log insn details here

   undefinedInstructionCommon();
}

void SymEvalPolicy_64::undefinedInstructionCommon() {
   for (std::map<Absloc, Assignment::Ptr>::iterator iter = aaMap.begin();
        iter != aaMap.end(); ++iter) {
      res[iter->second] = getBottomAST();
   }
   failedTranslate_ = true;
}

void SymEvalPolicy_64::startInstruction(SgAsmx86Instruction *) {
}

void SymEvalPolicy_64::finishInstruction(SgAsmx86Instruction *) {
}
void SymEvalPolicy_64::startInstruction(SgAsmPowerpcInstruction *) {
}

void SymEvalPolicy_64::finishInstruction(SgAsmPowerpcInstruction *) {
}


Absloc SymEvalPolicy_64::convert(PowerpcRegisterClass regtype, int regNum)
{
    // Fix this function when adding support for ppc64
    switch(regtype)
    {
            // PPC register classes (gpr/spr/whatever) are identical 32/64, and must maintain this
            //  invariant, so we're using 32 here
        case powerpc_regclass_gpr:
            return Absloc(MachRegister(ppc32::GPR | regNum | arch));
        case powerpc_regclass_spr:
            return Absloc(MachRegister(ppc32::SPR | regNum | arch));
        case powerpc_regclass_cr:
        {
            if(arch == Arch_ppc64)
            {
                if(regNum < 0) return Absloc(ppc64::cr);
                if(regNum > 7) {
                    assert(!"bad CR field");
                    return Absloc();
                }
                return Absloc(ppc64::cr0 + regNum);
            } else {
                if(regNum < 0) return Absloc(ppc32::cr);
                if(regNum > 7) {
                    assert(!"bad CR field");
                    return Absloc();
                }
                return Absloc(ppc32::cr0 + regNum);

            }
        }
        default:
            assert(!"unknown power register class!");
            return Absloc();

        case powerpc_regclass_unknown:break;
        case powerpc_regclass_fpr:break;
        case powerpc_regclass_fpscr:break;
        case powerpc_regclass_tbr:break;
        case powerpc_regclass_msr:break;
        case powerpc_regclass_sr:break;
        case powerpc_last_register_class:break;
    }
    assert(0);
    return Absloc();
}

Absloc SymEvalPolicy_64::convert(X86GeneralPurposeRegister r)
{

  MachRegister mreg;
  switch (r) {
    case x86_gpr_ax:
      mreg = x86_64::rax;
      break;
    case x86_gpr_cx:
      mreg = x86_64::rcx;
      break;
    case x86_gpr_dx:
      mreg = x86_64::rdx;
      break;
    case x86_gpr_bx:
      mreg = x86_64::rbx;
      break;
    case x86_gpr_sp:
      mreg = x86_64::rsp;
      break;
    case x86_gpr_bp:
      mreg = x86_64::rbp;
      break;
    case x86_gpr_si:
      mreg = x86_64::rsi;
      break;
    case x86_gpr_di:
      mreg = x86_64::rdi;
      break;
    case x86_gpr_r8:
      mreg = x86_64::r8;
      break;
    case x86_gpr_r9:
      mreg = x86_64::r9;
      break;
    case x86_gpr_r10:
      mreg = x86_64::r10;
      break;
    case x86_gpr_r11:
      mreg = x86_64::r11;
      break;
    case x86_gpr_r12:
      mreg = x86_64::r12;
      break;
    case x86_gpr_r13:
      mreg = x86_64::r13;
      break;
    case x86_gpr_r14:
      mreg = x86_64::r14;
      break;
    case x86_gpr_r15:
      mreg = x86_64::r15;
      break;
    default:
      break;
  }

  return Absloc(mreg);;
}

Absloc SymEvalPolicy_64::convert(X86SegmentRegister r)
{
  MachRegister mreg;
  switch (r) {
    case x86_segreg_es:
      mreg = x86_64::es;
      break;
    case x86_segreg_cs:
      mreg = x86_64::cs;
      break;
    case x86_segreg_ss:
      mreg = x86_64::ss;
      break;
    case x86_segreg_ds:
      mreg = x86_64::ds;
      break;
    case x86_segreg_fs:
      mreg = x86_64::fs;
      break;
    case x86_segreg_gs:
      mreg = x86_64::gs;
      break;
    default:
      break;
  }

  return Absloc(mreg);
}

Absloc SymEvalPolicy_64::convert(X86Flag f)
{
   switch (f) {
      case x86_flag_cf:
         return Absloc(x86_64::cf);
      case x86_flag_1:
         return Absloc(x86_64::flag1);
      case x86_flag_pf:
         return Absloc(x86_64::pf);
      case x86_flag_3:
         return Absloc(x86_64::flag3);
      case x86_flag_af:
         return Absloc(x86_64::af);
      case x86_flag_5:
         return Absloc(x86_64::flag5);
      case x86_flag_zf:
         return Absloc(x86_64::zf);
      case x86_flag_sf:
         return Absloc(x86_64::sf);
      case x86_flag_tf:
         return Absloc(x86_64::tf);
      case x86_flag_if:
         return Absloc(x86_64::if_);
      case x86_flag_df:
         return Absloc(x86_64::df);
      case x86_flag_of:
         return Absloc(x86_64::of);
      case x86_flag_iopl0:
         return Absloc(x86_64::FLAGC);
      case x86_flag_iopl1:
         return Absloc(x86_64::FLAGD);
      case x86_flag_nt:
         return Absloc(x86_64::nt_);
      case x86_flag_15:
         return Absloc(x86_64::FLAGF);
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

