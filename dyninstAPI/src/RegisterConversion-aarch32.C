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
#include "registerSpace.h"

#include <map>
#include <boost/assign/list_of.hpp>
#include "Register.h"
#include "dyn_regs.h"
#include "registerSpace.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

//#warning "This file is not verified yet!"
multimap<Register, MachRegister> regToMachReg32 = map_list_of
    (registerSpace::r0, aarch32::r0)
    ;

multimap<Register, MachRegister> regToMachReg64 = map_list_of
    (registerSpace::r0, aarch32::r0)
    // (registerSpace::r0,           aarch32::x0)
    // ..
    // (registerSpace::r30,          aarch32::x30)
    // (registerSpace::fpr0,         aarch32::q0)
    // ..
    // (registerSpace::fpr31,        aarch32::q31)
    // (registerSpace::lr,           aarch32::x30)
    // (registerSpace::sp,           aarch32::sp)
    // (registerSpace::pc,           aarch32::pc)
    // (registerSpace::pstate,       aarch32::pstate)
    // (registerSpace::fpcr,         aarch32::fpcr)
    // (registerSpace::fpsr,         aarch32::fpsr)
    ;

map<MachRegister, Register> reverseRegisterMap = map_list_of
    (aarch32::r0,  registerSpace::r0)
    // (aarch32::x0,  registerSpace::r0)
    // ..
    // (aarch32::x30, registerSpace::r30)
    // (aarch32::q0,   registerSpace::fpr0)
    // ..
    // (aarch32::q31,  registerSpace::fpr31)
    // (aarch32::x30,   registerSpace::lr)
    // (aarch32::sp,   registerSpace::sp)
    // (aarch32::pc,   registerSpace::pc)
    // (aarch32::pstate,   registerSpace::pstate)
    // (aarch32::fpcr,   registerSpace::fpcr)
    // (aarch32::fpsr,   registerSpace::fpsr)
    ;

Register convertRegID(MachRegister reg, bool &wasUpcast)
{
    wasUpcast = false;

    MachRegister baseReg = MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_aarch32);
//    RegisterAST::Ptr debug(new RegisterAST(baseReg));
//    fprintf(stderr, "DEBUG: converting %s", toBeConverted->format().c_str());
//    fprintf(stderr, " to %s\n", debug->format().c_str());
    map<MachRegister, Register>::const_iterator found =
      reverseRegisterMap.find(baseReg);
    if(found == reverseRegisterMap.end()) {
      // Yeah, this happens when we analyze trash code. Oops...
      return registerSpace::ignored;
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
      return registerSpace::ignored;
    }
    return convertRegID(toBeConverted->getID(), wasUpcast);
}

MachRegister convertRegID(Register r, Dyninst::Architecture arch) {
    if( arch == Arch_aarch32 ) {
        switch(r) {
            case registerSpace::r0:    return aarch32::r0;
            case registerSpace::r1:    return aarch32::r1;
            case registerSpace::r2:    return aarch32::r2;
            case registerSpace::r3:    return aarch32::r3;
            case registerSpace::r4:    return aarch32::r4;
            case registerSpace::r5:    return aarch32::r5;
            case registerSpace::r6:    return aarch32::r6;
            case registerSpace::r7:    return aarch32::r7;
            case registerSpace::r8:    return aarch32::r8;
            case registerSpace::r9:    return aarch32::r9;
            case registerSpace::r10:   return aarch32::r10;
            case registerSpace::r11:   return aarch32::r11;
            case registerSpace::r12:   return aarch32::r12;
            case registerSpace::r13:   return aarch32::r13;
            case registerSpace::r14:   return aarch32::r14;
            case registerSpace::r15:   return aarch32::r15;
            case registerSpace::fpr0:  return aarch32::q0;
            case registerSpace::fpr1:  return aarch32::q1;
            case registerSpace::fpr2:  return aarch32::q2;
            case registerSpace::fpr3:  return aarch32::q3;
            case registerSpace::fpr4:  return aarch32::q4;
            case registerSpace::fpr5:  return aarch32::q5;
            case registerSpace::fpr6:  return aarch32::q6;
            case registerSpace::fpr7:  return aarch32::q7;
            case registerSpace::fpr8:  return aarch32::q8;
            case registerSpace::fpr9:  return aarch32::q9;
            case registerSpace::fpr10: return aarch32::q10;
            case registerSpace::fpr11: return aarch32::q11;
            case registerSpace::fpr12: return aarch32::q12;
            case registerSpace::fpr13: return aarch32::q13;
            case registerSpace::fpr14: return aarch32::q14;
            case registerSpace::fpr15: return aarch32::q15;
            case registerSpace::lr: 	return aarch32::r3;
            case registerSpace::sp: 	return aarch32::sp;
            case registerSpace::pc: 	return aarch32::r15;
            case registerSpace::pstate: 	return aarch32::pstate;
            default:
                break;
        }
    }else{
        assert(!"Invalid architecture");
    }

    assert(!"Register not handled");
    return InvalidReg;
}
