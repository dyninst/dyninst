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
#include "dyninstAPI/src/debug.h"
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

bool PCWidget::PCtoReturnAddr(const codeGen &templ, const RelocBlock *t, CodeBuffer &buffer) {
  if(templ.addrSpace()->proc()) {
    std::vector<unsigned char> newInsn;
    // We want to get a value into LR, which is the return address.
    // Fun for the whole family... we need a spare register. Argh!
    codeGen gen(64);
    gen.applyTemplate(templ);
    // Must be in LR
    instPoint *point = templ.point();
    
    // If we do not have a point then we have to invent one
    if (!point || (point->type() != instPoint::PreInsn && point->insnAddr() != addr())) {
      point = instPoint::preInsn(t->func(), t->block(), addr(), insn_, true);
    }
    assert(point);  
    
    registerSpace *rs = registerSpace::actualRegSpace(point);
    gen.setRegisterSpace(rs);
    int stackSize = 0;
    std::vector<Register> freeReg;
    std::vector<Register> excludeReg;  
    
    Address origRet = addr() + insn_.size();
    Register scratch = gen.rs()->getScratchRegister(gen, true);
    bool createFrame = false;
    if (scratch == Null_Register) {
      stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
      assert(stackSize == 1);
      scratch = freeReg[0];
      createFrame = true;
    }
    insnCodeGen::loadImmIntoReg(gen, scratch, origRet);
    insnCodeGen::generateMoveToLR(gen, scratch);
    if (createFrame) {
      insnCodeGen::removeStackFrame(gen);
    }
    buffer.addPIC(gen, tracker(t));
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
    // Move immediate to register?
    codeGen gen(16);
    insnCodeGen::loadImmIntoReg(gen, reg, addr_);
    buffer.addPIC(gen, tracker(t));
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Reg, addr_, reg, thunkAddr_, insn_, t->block(), t->func());
    buffer.addPatch(newPatch, tracker(t));
  }
  return true;
}

#include "dyninstAPI/src/registerSpace.h"
bool IPPatch::apply(codeGen &gen, CodeBuffer *) {
  relocation_cerr << "\t\t IPPatch::apply" << endl;
  relocation_cerr << "\t\t\t Generating IPPatch for target address " << std::hex << addr << ", CodeGen current address " << std::hex << gen.currAddr() << " and register number " << reg << endl;

  // For dynamic we can do this in-line
  assert(gen.addrSpace()->edit());

  instPoint *point = gen.point();
  // If we do not have a point then we have to invent one
  if (!point || (point->type() != instPoint::PreInsn && point->insnAddr() != addr)) {
    point = instPoint::preInsn(func, block, addr, insn, true);
  }
  assert(point);
    
  registerSpace *rs = registerSpace::actualRegSpace(point);
  gen.setRegisterSpace(rs);

  // Must be in LR
  if (reg == (Register) -1) reg = registerSpace::lr;
  assert(reg == registerSpace::lr);
    
  int stackSize = 0;
  std::vector<Register> freeReg;
  std::vector<Register> excludeReg;
    
  Register scratchPCReg = gen.rs()->getScratchRegister(gen, true);
  excludeReg.push_back(scratchPCReg);
  Register scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
    
  if ((scratchPCReg == Null_Register) && (scratchReg == Null_Register)) {
    excludeReg.clear();
    stackSize = insnCodeGen::createStackFrame(gen, 2, freeReg, excludeReg);
    assert(stackSize == 2);
    scratchPCReg = freeReg[0];
    scratchReg = freeReg[1];
      
  } else if (scratchReg == Null_Register && scratchPCReg != Null_Register) {
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
  return true;
}
