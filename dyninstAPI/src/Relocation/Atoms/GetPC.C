/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include "GetPC.h"
#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/addressSpace.h" // For determining which type of getPC to emit
#include "dyninstAPI/src/RegisterConversion-x86.h"
#include "dyninstAPI/src/debug.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

////////////////////////
GetPC::Ptr GetPC::create(Instruction::Ptr insn,
			 Address addr,
			 Absloc a,
			 Address thunk) {
  return Ptr(new GetPC(insn, addr, a, thunk));
}

bool GetPC::generate(GenStack &gens) {
  // Two options: top of stack (push origAddr) 
  // or into a register (/w/ a mov)

  switch (a_.type()) {
  case Absloc::Stack:
    return PCtoStack(gens);
  case Absloc::Register:
    return PCtoReg(gens);
  default:
    cerr << "Error: getPC has unknown Absloc type " << a_.format() << endl;
    return false;
  }
}

bool GetPC::PCtoStack(GenStack &gens) {

  if(gens().addrSpace()->proc()) {
    GET_PTR(newInsn, gens());
    *newInsn = 0x68; // Push; we're replacing "call 0" with "push original IP"
    newInsn++;	  
    Address EIP = addr_ + insn_->size();
    unsigned int *temp = (unsigned int *) newInsn;
    *temp = EIP;
    // No 9-byte jumps...
    assert(sizeof(unsigned int) == 4); // should be a compile-time assert
    newInsn += sizeof(unsigned int);
    SET_PTR(newInsn, gens());
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Push, addr_ + insn_->size());
    gens.addPatch(newPatch);
  }	

  return true;
}

bool GetPC::PCtoReg(GenStack &gens) {

  bool ignored;
  Register reg = convertRegID(a_.reg(), ignored);

  if(gens().addrSpace()->proc()) {
    GET_PTR(newInsn, gens());
    // Okay, put the PC into the 'reg'
    Address EIP = addr_ + insn_->size();
    *newInsn = static_cast<unsigned char>(0xb8 + reg); 
    // MOV family, destination of the register encoded by
    // 'reg', source is an Iv immediate
    newInsn++;
    unsigned int *temp = (unsigned int *)newInsn;
    *temp = EIP;
    //assert(sizeof(unsigned int *)==4);
    //newInsn += sizeof(unsigned int *);
    newInsn += 4;  // fix for AMD64
    SET_PTR(newInsn, gens());
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Reg, addr_ + insn_->size(), reg, thunkAddr_);
    gens.addPatch(newPatch);
  }
  return true;
}

string GetPC::format() const {
  stringstream ret;
  ret << "GetPC(" 
      << std::hex << addr_ << std::dec;
  ret << "" << a_.format();
  return ret.str();
}

#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/inst-x86.h"

bool IPPatch::apply(codeGen &gen, int, int) {
  relocation_cerr << "\t\t IPPatch::apply" << endl;

  // We want to generate orig_value into the appropriate location.

  if (0 && (type == Reg) && thunk) {
    // Let's try this...
    insnCodeGen::generateCall(gen,
			      gen.currAddr(),
			      thunk); 
    // Okay, now we'll be getting back the wrong number.
    Address retAddr = gen.currAddr();
    if (type != Reg) {
      cerr << "Aborting at IPPatch, orig_value " << hex << orig_value << " and thunk " << thunk << endl;
    }
    assert(type == Reg);
    emitLEA(RealRegister(reg),
	    RealRegister(Null_Register),
	    0,
	    orig_value - retAddr,
	    RealRegister(reg),
	    gen);
  }
  else {
    GET_PTR(newInsn, gen); 
    
    *newInsn = 0xE8;
    newInsn++;
    unsigned int *temp = (uint32_t *) newInsn;
    *temp = 0;
    newInsn += sizeof(uint32_t);
    SET_PTR(newInsn, gen);
    Address offset = orig_value - gen.currAddr();
    REGET_PTR(newInsn, gen);
    *newInsn = 0x81;
    newInsn++;
    *newInsn = 0x04;
    newInsn++;
    *newInsn = 0x24;
    newInsn++;
    temp =  (uint32_t *) newInsn;
    *temp = offset;
    newInsn += sizeof(uint32_t);	  

    if (type == Reg) {
      assert(reg != (Register) -1);
      // pop...
      *newInsn++ = static_cast<unsigned char>(0x58 + reg); // POP family
    }
    SET_PTR(newInsn, gen);
  }

  return true;
}

bool IPPatch::preapply(codeGen &gen) {
  // It's gonna be a big one...
  gen.moveIndex(1+4+1+1+1+4 + ((type == Reg) ? 1 : 0));
  return true;
}

