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

#include "PCAtom.h"
#include "instructionAPI/h/Instruction.h"
#include "../patchapi_debug.h"
#include "Trace.h"
#include "../CodeBuffer.h"
#include "../CodeTracker.h"
#include "dyninstAPI/src/function.h"

#include "dyninstAPI/src/addressSpace.h" // For determining which type of getPC to emit
#include "dyninstAPI/src/RegisterConversion-x86.h"


using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

////////////////////////
PCAtom::Ptr PCAtom::create(Instruction::Ptr insn,
			 Address addr,
			 Absloc a,
			 Address thunk) {
  return Ptr(new PCAtom(insn, addr, a, thunk));
}

TrackerElement *PCAtom::tracker(block_instance *b) const {
  assert(addr_ != 1);
  EmulatorTracker *e = new EmulatorTracker(addr_, b);
  return e;
}

bool PCAtom::generate(const codeGen &templ, const Trace *trace, CodeBuffer &buffer) {
  // Two options: top of stack (push origAddr) 
  // or into a register (/w/ a mov)

  switch (a_.type()) {
  case Absloc::Stack:
     return PCtoStack(templ, trace, buffer);
  case Absloc::Register:
     return PCtoReg(templ, trace, buffer);
  default:
    cerr << "Error: getPC has unknown Absloc type " << a_.format() << endl;
    return false;
  }
}

bool PCAtom::PCtoStack(const codeGen &templ, const Trace *t, CodeBuffer &buffer) {
   if(templ.addrSpace()->proc()) {
      std::vector<unsigned char> newInsn;
      newInsn.push_back(0x68); // push
      Address EIP = addr_ + insn_->size();
      unsigned char *tmp = (unsigned char *) &EIP;
      newInsn.insert(newInsn.end(),
                     tmp,
                     tmp+sizeof(unsigned int));
      buffer.addPIC(newInsn, tracker(t->block()));
   }
   else {
      IPPatch *newPatch = new IPPatch(IPPatch::Push, addr_ + insn_->size());
      buffer.addPatch(newPatch, tracker(t->block()));
   }	
   
  return true;
}

bool PCAtom::PCtoReg(const codeGen &templ, const Trace *t, CodeBuffer &buffer) {
  bool ignored;
  Register reg = convertRegID(a_.reg(), ignored);

  if(templ.addrSpace()->proc()) {
     std::vector<unsigned char> newInsn;
     newInsn.push_back(static_cast<unsigned char>(0xb8 + reg));
     // MOV family, destination of the register encoded by
     // 'reg', source is an Iv immediate
     
     Address EIP = addr_ + insn_->size();
     unsigned char *tmp = (unsigned char *) &EIP;
     newInsn.insert(newInsn.end(),
                    tmp,
                    tmp + sizeof(unsigned int));
     buffer.addPIC(newInsn, tracker(t->block()));
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Reg, addr_ + insn_->size(), reg, thunkAddr_);
    buffer.addPatch(newPatch, tracker(t->block()));
  }
  return true;
}

string PCAtom::format() const {
  stringstream ret;
  ret << "PCAtom(" 
      << std::hex << addr_ << std::dec;
  ret << "" << a_.format();
  return ret.str();
}

#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/inst-x86.h"

bool IPPatch::apply(codeGen &gen, CodeBuffer *) {
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

unsigned IPPatch::estimate(codeGen &) {
   // should be the minimum required since we expand
   // but never contract

   // In the process case we always just generate it 
   // straight out, because we know the original address.

   // It's gonna be a big one...
   return 1+4+1+1+1+4 + ((type == Reg) ? 1 : 0);
}

