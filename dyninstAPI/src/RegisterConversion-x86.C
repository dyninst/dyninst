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

#include "RegisterConversion.h"
#include "inst-x86.h"

#include <map>
#include <boost/assign/list_of.hpp>
#include "Register.h"
#include "dyn_regs.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;
using namespace NS_x86;

multimap<Register, MachRegister> regToMachReg32 = map_list_of
(REGNUM_EAX, x86::eax)
(REGNUM_ECX, x86::ecx)
(REGNUM_EDX, x86::edx)
(REGNUM_EBX, x86::ebx)
(REGNUM_ESP, x86::esp)
(REGNUM_EBP, x86::ebp)
(REGNUM_ESI, x86::esi)
(REGNUM_EDI, x86::edi)
(REGNUM_IGNORED,x86::cs)
(REGNUM_IGNORED,x86::ds)
(REGNUM_IGNORED,x86::es)
(REGNUM_IGNORED,x86::fs)
(REGNUM_IGNORED,x86::gs)
(REGNUM_IGNORED,x86::ss)
(REGNUM_IGNORED,x86::eip)
(REGNUM_CF,x86::cf)
(REGNUM_PF,x86::pf)
(REGNUM_AF,x86::af)
(REGNUM_ZF,x86::zf)
(REGNUM_SF,x86::sf)
(REGNUM_TF,x86::tf)
(REGNUM_DF,x86::df)
(REGNUM_OF,x86::of)
(REGNUM_NT,x86::nt_)
(REGNUM_IF,x86::if_)
(REGNUM_RF,x86::rf)
(REGNUM_EFLAGS,x86::cf)
(REGNUM_EFLAGS,x86::pf)
(REGNUM_EFLAGS,x86::af)
(REGNUM_EFLAGS,x86::zf)
(REGNUM_EFLAGS,x86::sf)
(REGNUM_EFLAGS,x86::tf)
(REGNUM_EFLAGS,x86::df)
(REGNUM_EFLAGS,x86::of)
(REGNUM_EFLAGS,x86::nt_)
(REGNUM_EFLAGS,x86::if_)
(REGNUM_EFLAGS,x86::rf)

(REGNUM_XMM0,x86::xmm0)
(REGNUM_XMM1,x86::xmm1)
(REGNUM_XMM2,x86::xmm2)
(REGNUM_XMM3,x86::xmm3)
(REGNUM_XMM4,x86::xmm4)
(REGNUM_XMM5,x86::xmm5)
(REGNUM_XMM6,x86::xmm6)
(REGNUM_XMM7,x86::xmm7)
(REGNUM_DUMMYFPR,x86::mm0)
(REGNUM_MM0,x86::mm0)
(REGNUM_MM1,x86::mm0)
(REGNUM_MM2,x86::mm0)
(REGNUM_MM3,x86::mm0)
(REGNUM_MM4,x86::mm0)
(REGNUM_MM5,x86::mm0)
(REGNUM_MM6,x86::mm0)
(REGNUM_MM7,x86::mm0)

(REGNUM_IGNORED,x86::cr0)
(REGNUM_IGNORED,x86::cr1)
(REGNUM_IGNORED,x86::cr2)
(REGNUM_IGNORED,x86::cr3)
(REGNUM_IGNORED,x86::cr4)
(REGNUM_IGNORED,x86::cr5)
(REGNUM_IGNORED,x86::cr6)
(REGNUM_IGNORED,x86::cr7)
(REGNUM_IGNORED,x86::dr0)
(REGNUM_IGNORED,x86::dr1)
(REGNUM_IGNORED,x86::dr2)
(REGNUM_IGNORED,x86::dr3)
(REGNUM_IGNORED,x86::dr4)
(REGNUM_IGNORED,x86::dr5)
(REGNUM_IGNORED,x86::dr6)
(REGNUM_IGNORED,x86::dr7)
;

multimap<Register, MachRegister> regToMachReg64 = map_list_of
(REGNUM_R8,x86_64::r8)
(REGNUM_R9,x86_64::r9)
(REGNUM_R10,x86_64::r10)
(REGNUM_R11,x86_64::r11)
(REGNUM_R12,x86_64::r12)
(REGNUM_R13,x86_64::r13)
(REGNUM_R14,x86_64::r14)
(REGNUM_R15,x86_64::r15)
(REGNUM_IGNORED,x86_64::cs)
(REGNUM_IGNORED,x86_64::ds)
(REGNUM_IGNORED,x86_64::es)
(REGNUM_IGNORED,x86_64::fs)
(REGNUM_IGNORED,x86_64::gs)
(REGNUM_IGNORED,x86_64::ss)
(REGNUM_RAX,x86_64::rax)
(REGNUM_RCX,x86_64::rcx)
(REGNUM_RDX,x86_64::rdx)
(REGNUM_RBX,x86_64::rbx)
(REGNUM_RSP,x86_64::rsp)
(REGNUM_RBP,x86_64::rbp)
(REGNUM_RSI,x86_64::rsi)
(REGNUM_RDI,x86_64::rdi)
(REGNUM_IGNORED,x86_64::rip)
(REGNUM_CF,x86_64::cf)
(REGNUM_PF,x86_64::pf)
(REGNUM_AF,x86_64::af)
(REGNUM_ZF,x86_64::zf)
(REGNUM_SF,x86_64::sf)
(REGNUM_TF,x86_64::tf)
(REGNUM_DF,x86_64::df)
(REGNUM_OF,x86_64::of)
(REGNUM_NT,x86_64::nt_)
(REGNUM_IF,x86_64::if_)
(REGNUM_RF,x86_64::rf)
(REGNUM_EFLAGS,x86_64::cf)
(REGNUM_EFLAGS,x86_64::pf)
(REGNUM_EFLAGS,x86_64::af)
(REGNUM_EFLAGS,x86_64::zf)
(REGNUM_EFLAGS,x86_64::sf)
(REGNUM_EFLAGS,x86_64::tf)
(REGNUM_EFLAGS,x86_64::df)
(REGNUM_EFLAGS,x86_64::of)
(REGNUM_EFLAGS,x86_64::nt_)
(REGNUM_EFLAGS,x86_64::if_)
(REGNUM_EFLAGS,x86_64::rf)
(REGNUM_XMM0,x86_64::xmm0)
(REGNUM_XMM1,x86_64::xmm1)
(REGNUM_XMM2,x86_64::xmm2)
(REGNUM_XMM3,x86_64::xmm3)
(REGNUM_XMM4,x86_64::xmm4)
(REGNUM_XMM5,x86_64::xmm5)
(REGNUM_XMM6,x86_64::xmm6)
(REGNUM_XMM7,x86_64::xmm7)
(REGNUM_XMM8,x86_64::xmm8)
(REGNUM_XMM9,x86_64::xmm9)
(REGNUM_XMM10,x86_64::xmm10)
(REGNUM_XMM11,x86_64::xmm11)
(REGNUM_XMM12,x86_64::xmm12)
(REGNUM_XMM13,x86_64::xmm13)
(REGNUM_XMM14,x86_64::xmm14)
(REGNUM_XMM15,x86_64::xmm15)
(REGNUM_DUMMYFPR,x86_64::mm0)
(REGNUM_MM0, x86_64::mm0)
(REGNUM_MM1, x86_64::mm0)
(REGNUM_MM2, x86_64::mm0)
(REGNUM_MM3, x86_64::mm0)
(REGNUM_MM4, x86_64::mm0)
(REGNUM_MM5, x86_64::mm0)
(REGNUM_MM6, x86_64::mm0)
(REGNUM_MM7, x86_64::mm0)
(REGNUM_IGNORED,x86_64::cr0)
(REGNUM_IGNORED,x86_64::cr1)
(REGNUM_IGNORED,x86_64::cr2)
(REGNUM_IGNORED,x86_64::cr3)
(REGNUM_IGNORED,x86_64::cr4)
(REGNUM_IGNORED,x86_64::cr5)
(REGNUM_IGNORED,x86_64::cr6)
(REGNUM_IGNORED,x86_64::cr7)
(REGNUM_IGNORED,x86_64::dr0)
(REGNUM_IGNORED,x86_64::dr1)
(REGNUM_IGNORED,x86_64::dr2)
(REGNUM_IGNORED,x86_64::dr3)
(REGNUM_IGNORED,x86_64::dr4)
(REGNUM_IGNORED,x86_64::dr5)
(REGNUM_IGNORED,x86_64::dr6)
(REGNUM_IGNORED,x86_64::dr7)
;


map<MachRegister, Register> reverseRegisterMap = map_list_of
        (x86_64::r8, REGNUM_R8)
        (x86_64::r9, REGNUM_R9)
        (x86_64::r10, REGNUM_R10)
        (x86_64::r11, REGNUM_R11)
        (x86_64::r12, REGNUM_R12)
        (x86_64::r13, REGNUM_R13)
        (x86_64::r14, REGNUM_R14)
        (x86_64::r15, REGNUM_R15)
        (x86_64::cs, REGNUM_IGNORED)
        (x86_64::ds, REGNUM_IGNORED)
        (x86_64::es, REGNUM_IGNORED)
        (x86_64::fs, REGNUM_IGNORED)
        (x86_64::gs, REGNUM_IGNORED)
        (x86_64::ss, REGNUM_IGNORED)
        (x86_64::rax, REGNUM_RAX)
        (x86_64::rcx, REGNUM_RCX)
        (x86_64::rdx, REGNUM_RDX)
        (x86_64::rbx, REGNUM_RBX)
        (x86_64::rsp, REGNUM_RSP)
        (x86_64::rbp, REGNUM_RBP)
        (x86_64::rsi, REGNUM_RSI)
        (x86_64::rdi, REGNUM_RDI)
        (x86_64::rip, REGNUM_IGNORED)
        (x86_64::cf, REGNUM_CF)
        (x86_64::pf, REGNUM_PF)
        (x86_64::af, REGNUM_AF)
        (x86_64::zf, REGNUM_ZF)  
        (x86_64::sf, REGNUM_SF)
        (x86_64::tf, REGNUM_TF)
        (x86_64::df, REGNUM_DF)
        (x86_64::of, REGNUM_OF)
        (x86_64::nt_, REGNUM_NT)
        (x86_64::if_, REGNUM_IF)
        (x86_64::flags, REGNUM_EFLAGS)
        (x86_64::xmm0, REGNUM_XMM0)
        (x86_64::xmm1, REGNUM_XMM1)
        (x86_64::xmm2, REGNUM_XMM2)
        (x86_64::xmm3, REGNUM_XMM3)
        (x86_64::xmm4, REGNUM_XMM4)
        (x86_64::xmm5, REGNUM_XMM5)
        (x86_64::xmm6, REGNUM_XMM6)
        (x86_64::xmm7, REGNUM_XMM7)
        (x86_64::xmm8, REGNUM_XMM8)
        (x86_64::xmm9, REGNUM_XMM9)
        (x86_64::xmm10, REGNUM_XMM10)
        (x86_64::xmm11, REGNUM_XMM11)
        (x86_64::xmm12, REGNUM_XMM12)
        (x86_64::xmm13, REGNUM_XMM13)
        (x86_64::xmm14, REGNUM_XMM14)
        (x86_64::xmm15, REGNUM_XMM15)
        (x86_64::mm0, REGNUM_DUMMYFPR)
        (x86_64::mm1, REGNUM_DUMMYFPR)
        (x86_64::mm2, REGNUM_DUMMYFPR)
        (x86_64::mm3, REGNUM_DUMMYFPR)
        (x86_64::mm4, REGNUM_DUMMYFPR)
        (x86_64::mm5, REGNUM_DUMMYFPR)
        (x86_64::mm6, REGNUM_DUMMYFPR)
        (x86_64::mm7, REGNUM_DUMMYFPR)
        (x86_64::cr0, REGNUM_IGNORED)
        (x86_64::cr1, REGNUM_IGNORED)
        (x86_64::cr2, REGNUM_IGNORED)
        (x86_64::cr3, REGNUM_IGNORED)
        (x86_64::cr4, REGNUM_IGNORED)
        (x86_64::cr5, REGNUM_IGNORED)
        (x86_64::cr6, REGNUM_IGNORED)
        (x86_64::cr7, REGNUM_IGNORED)
        (x86_64::dr0, REGNUM_IGNORED)
        (x86_64::dr1, REGNUM_IGNORED)
        (x86_64::dr2, REGNUM_IGNORED)
        (x86_64::dr3, REGNUM_IGNORED)
        (x86_64::dr4, REGNUM_IGNORED)
        (x86_64::dr5, REGNUM_IGNORED)
        (x86_64::dr6, REGNUM_IGNORED)
        (x86_64::dr7, REGNUM_IGNORED)
        (x86_64::st0, REGNUM_DUMMYFPR)
        (x86_64::st1, REGNUM_DUMMYFPR)
        (x86_64::st2, REGNUM_DUMMYFPR)
        (x86_64::st3, REGNUM_DUMMYFPR)
        (x86_64::st4, REGNUM_DUMMYFPR)
        (x86_64::st5, REGNUM_DUMMYFPR)
        (x86_64::st6, REGNUM_DUMMYFPR)
        (x86_64::st7, REGNUM_DUMMYFPR)
        ;

Register convertRegID(MachRegister reg, bool &wasUpcast) {
    wasUpcast = false;
    if(reg.getBaseRegister().val() != reg.val()) wasUpcast = true;
    MachRegister baseReg = MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_x86_64);
//    RegisterAST::Ptr debug(new RegisterAST(baseReg));
//    fprintf(stderr, "DEBUG: converting %s", toBeConverted->format().c_str());
//    fprintf(stderr, " to %s\n", debug->format().c_str());
    map<MachRegister, Register>::const_iterator found =
            reverseRegisterMap.find(baseReg);
    if(found == reverseRegisterMap.end()) {
      // Yeah, this happens when we analyze trash code. Oops...
		return REGNUM_IGNORED;
    }
    if(found->second == REGNUM_DUMMYFPR) {
        wasUpcast = true;
        if(reg.getArchitecture() == Arch_x86)
        {
            return IA32_FPR_VIRTUAL_REGISTER;
        }
    }
    return found->second;
}


Register convertRegID(RegisterAST::Ptr toBeConverted, bool& wasUpcast)
{
    return convertRegID(toBeConverted.get(), wasUpcast);
}
        
Register convertRegID(RegisterAST* toBeConverted, bool& wasUpcast)
{
    if(!toBeConverted) {
        //assert(0);
        return REGNUM_IGNORED;
    }
    return convertRegID(toBeConverted->getID(), wasUpcast);
}

map<Register, MachRegister> machRegisterMapx86_64 = map_list_of
        (REGNUM_R8, x86_64::r8)
        (REGNUM_R9, x86_64::r9)
        (REGNUM_R10, x86_64::r10)
        (REGNUM_R11, x86_64::r11)
        (REGNUM_R12, x86_64::r12)
        (REGNUM_R13, x86_64::r13)
        (REGNUM_R14, x86_64::r14)
        (REGNUM_R15, x86_64::r15)
        (REGNUM_RAX, x86_64::rax)
        (REGNUM_RCX, x86_64::rcx)
        (REGNUM_RDX, x86_64::rdx)
        (REGNUM_RBX, x86_64::rbx)
        (REGNUM_RSP, x86_64::rsp)
        (REGNUM_RBP, x86_64::rbp)
        (REGNUM_RSI, x86_64::rsi)
        (REGNUM_RDI, x86_64::rdi)
        (REGNUM_CF, x86_64::cf)
        (REGNUM_PF, x86_64::pf)
        (REGNUM_AF, x86_64::af)
        (REGNUM_ZF, x86_64::zf)  
        (REGNUM_SF, x86_64::sf)
        (REGNUM_TF, x86_64::tf)
        (REGNUM_DF, x86_64::df)
        (REGNUM_OF, x86_64::of)
        (REGNUM_NT, x86_64::nt_)
        (REGNUM_IF, x86_64::if_)
        (REGNUM_EFLAGS, x86_64::flags)
        (REGNUM_XMM0, x86_64::xmm0)
        (REGNUM_XMM1, x86_64::xmm1)
        (REGNUM_XMM2, x86_64::xmm2)
        (REGNUM_XMM3, x86_64::xmm3)
        (REGNUM_XMM4, x86_64::xmm4)
        (REGNUM_XMM5, x86_64::xmm5)
        (REGNUM_XMM6, x86_64::xmm6)
        (REGNUM_XMM7, x86_64::xmm7)
        ;

map<Register, MachRegister> machRegisterMapx86 = map_list_of
        (REGNUM_EAX, x86::eax)
        (REGNUM_ECX, x86::ecx)
        (REGNUM_EDX, x86::edx)
        (REGNUM_EBX, x86::ebx)
        (REGNUM_ESP, x86::esp)
        (REGNUM_EBP, x86::ebp)
        (REGNUM_ESI, x86::esi)
        (REGNUM_EDI, x86::edi)
        (REGNUM_CF, x86::cf)
        (REGNUM_PF, x86::pf)
        (REGNUM_AF, x86::af)
        (REGNUM_ZF, x86::zf)  
        (REGNUM_SF, x86::sf)
        (REGNUM_TF, x86::tf)
        (REGNUM_DF, x86::df)
        (REGNUM_OF, x86::of)
        (REGNUM_NT, x86::nt_)
        (REGNUM_IF, x86::if_)
        (REGNUM_EFLAGS, x86::flags)
        (REGNUM_XMM0, x86::xmm0)
        (REGNUM_XMM1, x86::xmm1)
        (REGNUM_XMM2, x86::xmm2)
        (REGNUM_XMM3, x86::xmm3)
        (REGNUM_XMM4, x86::xmm4)
        (REGNUM_XMM5, x86::xmm5)
        (REGNUM_XMM6, x86::xmm6)
        (REGNUM_XMM7, x86::xmm7)
        ;


MachRegister convertRegID(Register reg, Dyninst::Architecture arch) {
    map<Register, MachRegister>::const_iterator found;
    switch(arch) {
        case Arch_x86_64:
            found = machRegisterMapx86_64.find(reg);
            assert(    found != machRegisterMapx86_64.end() 
                    && "No Register->MachRegister mapping found" );
            break;
        case Arch_x86:
            found = machRegisterMapx86.find(reg);
            assert(    found != machRegisterMapx86.end() 
                    && "No Register->MachRegister mapping found" );
            break;
        default:
            assert(!"Invalid architecture");
            break;
    }

    return found->second;
}
