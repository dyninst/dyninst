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
    // We want to move a value into LR (x30). 
    // On ARM, LR is just a normal GPR. We can directly move the value into it
    codeGen gen(16);
    gen.applyTemplate(templ);
    Address origRet = addr() + insn_.size();
    insnCodeGen::loadImmIntoReg(gen, 30 /* LR */, origRet);
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
#include "dyninstAPI/src/emit-aarch64.h"
bool IPPatch::apply(codeGen &gen, CodeBuffer *) {
  relocation_cerr << "\t\t IPPatch::apply" << endl;
  relocation_cerr << "\t\t\t Generating IPPatch for target address " << std::hex << addr << ", CodeGen current address " << std::hex << gen.currAddr() << " and register number " << reg << endl;

  // We want to generate addr (as modified) into the appropriate location.
  // TODO get rid of the #ifdef here...

  instPoint *point = gen.point();
  // If we do not have a point then we have to invent one
  if (!point || (point->type() != instPoint::PreInsn && point->insnAddr() != addr)) {
    point = instPoint::preInsn(func, block, addr, insn, true);
  }
  assert(point);
    
  registerSpace *rs = registerSpace::actualRegSpace(point);
  gen.setRegisterSpace(rs);

  // Calculate the offset between current PC and original RA
  EmitterAARCH64* emitter = static_cast<EmitterAARCH64*>(gen.emitter());
  Address RAOffset = addr - emitter->emitMovePCToReg( 30 /* LR */ , gen) + 4;
  // Load the offset into a scratch register
  std::vector<Register> exclude;
  exclude.push_back(30 /* LR */);
  Register scratchReg = insnCodeGen::moveValueToReg(gen, labs(RAOffset), &exclude);
  // Put the original RA into LR
  insnCodeGen::generateAddSubShifted(gen, RAOffset>0?insnCodeGen::Add:insnCodeGen::Sub,
          0, 0, scratchReg, 30, 30, true);
  // Do a jump to the actual target (so do not overwrite LR)
  insnCodeGen::generateBranch(gen, gen.currAddr(), addr, false);
  return true;
}
