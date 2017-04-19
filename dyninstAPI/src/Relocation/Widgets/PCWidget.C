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

#include "PCWidget.h"
#include "instructionAPI/h/Instruction.h"
#include "../dyninstAPI/src/debug.h"
#include "../CFG/RelocBlock.h"
#include "../CodeBuffer.h"
#include "../CodeTracker.h"
#include "dyninstAPI/src/function.h"

#include "dyninstAPI/src/addressSpace.h" // For determining which type of getPC to emit
#include "dyninstAPI/src/RegisterConversion.h"
#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/src/emitter.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

////////////////////////
PCWidget::Ptr PCWidget::create(Instruction::Ptr insn,
			 Address addr,
			 Absloc a,
			 Address thunk) {
  return Ptr(new PCWidget(insn, addr, a, thunk));
}

TrackerElement *PCWidget::tracker(const RelocBlock *t) const {
  assert(addr_ != 1);
  EmulatorTracker *e = new EmulatorTracker(addr_, t->block(), t->func());
  return e;
}

bool PCWidget::generate(const codeGen &templ, const RelocBlock *trace, CodeBuffer &buffer) {
  // Two options: top of stack (push origAddr) 
  // or into a register (/w/ a mov)

  switch (a_.type()) {
  case Absloc::Stack:
     return PCtoReturnAddr(templ, trace, buffer);
  case Absloc::Register:
     return PCtoReg(templ, trace, buffer);
  default:
    cerr << "Error: getPC has unknown Absloc type " << a_.format() << endl;
    return false;
  }
}

bool PCWidget::PCtoReturnAddr(const codeGen &templ, const RelocBlock *t, CodeBuffer &buffer) {
  if(templ.addrSpace()->proc()) {
    std::vector<unsigned char> newInsn;
#if defined(arch_x86) || defined(arch_x86_64)
    newInsn.push_back(0x68); // push
    Address EIP = addr_ + insn_->size();
    unsigned char *tmp = (unsigned char *) &EIP;
    newInsn.insert(newInsn.end(),
		   tmp,
		   tmp+sizeof(unsigned int));
    buffer.addPIC(newInsn, tracker(t));
#else
    // We want to get a value into LR, which is the return address.
    // Fun for the whole family... we need a spare register. Argh!

    codeGen gen(16);
    gen.applyTemplate(templ);
    // Must be in LR
    instPoint *point = templ.point();
    // If we do not have a point then we have to invent one
    if (!point || 
	(point->type() != instPoint::PreInsn &&
	 point->insnAddr() != addr())) {
      point = instPoint::preInsn(t->func(), t->block(), addr(), insn(), true);
    }
    assert(point);
    
    registerSpace *rs = registerSpace::actualRegSpace(point);
    gen.setRegisterSpace(rs);
    int stackSize = 0;
    pdvector<Register> freeReg;
    pdvector<Register> excludeReg;
    
    Address origRet = addr() + insn()->size();
    Register scratch = gen.rs()->getScratchRegister(gen, true);
    if (scratch == REG_NULL) {
      stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
      assert(stackSize == 1);
      scratch = freeReg[0];
    }
    insnCodeGen::loadImmIntoReg(gen, scratch, origRet);
    insnCodeGen::generateMoveToLR(gen, scratch);
    buffer.addPIC(gen, tracker(t));
#endif
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Push, addr_, insn_, t->block(), t->func());
    buffer.addPatch(newPatch, tracker(t));
  }	
   
  return true;
}

bool PCWidget::PCtoReg(const codeGen &templ, const RelocBlock *t, CodeBuffer &buffer) {
  bool ignored;
  Register reg = convertRegID(a_.reg(), ignored);

  if(templ.addrSpace()->proc()) {
#if defined(arch_x86) || defined(arch_x86_64)
     std::vector<unsigned char> newInsn;
     newInsn.push_back(static_cast<unsigned char>(0xb8 + reg));
     // MOV family, destination of the register encoded by
     // 'reg', source is an Iv immediate
     
     Address EIP = addr_ + insn_->size();
     unsigned char *tmp = (unsigned char *) &EIP;
     newInsn.insert(newInsn.end(),
                    tmp,
                    tmp + sizeof(unsigned int));
     buffer.addPIC(newInsn, tracker(t));
#else
     // Move immediate to register?
     codeGen gen(16);
     insnCodeGen::loadImmIntoReg(gen, reg, addr_);
     buffer.addPIC(gen, tracker(t));
#endif
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Reg, addr_, reg, thunkAddr_, insn_, t->block(), t->func());
    buffer.addPatch(newPatch, tracker(t));
  }
  return true;
}

string PCWidget::format() const {
  stringstream ret;
  ret << "PCWidget(" 
      << std::hex << addr_ << std::dec;
  ret << "" << a_.format();
  return ret.str();
}

#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/inst-x86.h"

bool IPPatch::apply(codeGen &gen, CodeBuffer *) {
  relocation_cerr << "\t\t IPPatch::apply" << endl;
  relocation_cerr << "\t\t\t Generating IPPatch for target address " << std::hex << addr << ", CodeGen current address " << std::hex << gen.currAddr() << " and register number " << reg << endl;

  // We want to generate addr (as modified) into the appropriate location.
  // TODO get rid of the #ifdef here...

#if defined(arch_x86) || defined(arch_x86_64) 
    GET_PTR(newInsn, gen); 
    
    *newInsn = 0xE8;
    newInsn++;
    unsigned int *temp = (uint32_t *) newInsn;
    *temp = 0;
    newInsn += sizeof(uint32_t);
    SET_PTR(newInsn, gen);
    Address offset = addr - gen.currAddr() + insn->size();
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
#else
    // For dynamic we can do this in-line
    assert(gen.addrSpace()->edit());

    instPoint *point = gen.point();
    // If we do not have a point then we have to invent one
    if (!point || 
	(point->type() != instPoint::PreInsn &&
	 point->insnAddr() != addr)) {
      point = instPoint::preInsn(func, block, addr, insn, true);
    }
    assert(point);
    
    registerSpace *rs = registerSpace::actualRegSpace(point);
    gen.setRegisterSpace(rs);

#if defined(arch_aarch64)
    instruction adrInsn;
    adrInsn.clear();

    if(reg == (Register)-1) {
	reg = gen.rs()->getScratchRegister(gen, true); 
    }

    INSN_SET(adrInsn, 28, 28, 0x1);
    INSN_SET(adrInsn, 0, 4, reg);
    insnCodeGen::generate(gen, adrInsn);

    long int offset = addr - gen.currAddr() + 4;
    pdvector<Register> exclude;
    exclude.push_back(reg);
    Register scratchReg = insnCodeGen::moveValueToReg(gen, offset, &exclude);
    insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Add, 0, 0, scratchReg, reg, reg, true);
#else
    // Must be in LR
    if (reg == (Register) -1) reg = registerSpace::lr;
    assert(reg == registerSpace::lr);
    
    int stackSize = 0;
    pdvector<Register> freeReg;
    pdvector<Register> excludeReg;
    
    Register scratchPCReg = gen.rs()->getScratchRegister(gen, true);
    excludeReg.push_back(scratchPCReg);
    Register scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
    
    if ((scratchPCReg == REG_NULL) && (scratchReg == REG_NULL)) {
      excludeReg.clear();
      stackSize = insnCodeGen::createStackFrame(gen, 2, freeReg, excludeReg);
      assert(stackSize == 2);
      scratchPCReg = freeReg[0];
      scratchReg = freeReg[1];
      
    } else if (scratchReg == REG_NULL && scratchPCReg != REG_NULL) {
      stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
      assert(stackSize == 1);
      scratchReg = freeReg[0];
    } 
    
    //scratchPCReg == NULL && scratchReg != NULL - not a valid case 
    //since getScratchRegister works in order
    
    // relocaAddr may have moved if we added instructions to setup a new stack frame
    Address newRelocAddr = gen.currAddr();
    
    insnCodeGen::generateBranch(gen, gen.currAddr(),  gen.currAddr()+4, true); // blrl
    insnCodeGen::generateMoveFromLR(gen, scratchPCReg); // mflr
    
    Address varOffset = addr - newRelocAddr;
    gen.emitter()->emitCallRelative(scratchReg, varOffset, scratchPCReg, gen);
    insnCodeGen::generateMoveToLR(gen, scratchReg);

    if( stackSize > 0) {
      insnCodeGen::removeStackFrame(gen); 
    }
#endif
#endif
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

