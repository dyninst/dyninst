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

#include <dyninstAPI/src/registerSpace.h>
#include "RelDataWidget.h"
#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/debug.h"
#include "CFG.h"
#include "../CFG/RelocBlock.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

RelDataWidget::Ptr RelDataWidget::create(Instruction insn,
                                         Address addr,
                                         Address target) {
  assert(addr);
  return Ptr(new RelDataWidget(insn, addr, target));
}

TrackerElement *RelDataWidget::tracker(const RelocBlock *t) const {
   EmulatorTracker *e = new EmulatorTracker(addr_, t->block(), t->func());
  return e;
}

bool RelDataWidget::generate(const codeGen &,
                              const RelocBlock *t, 
                              CodeBuffer &buffer) {
  // We want to take the original instruction and emulate
  // it at whatever our new address is. 

  // Fortunately, we can reuse old code to handle the
  // translation

  // Find the original target of the instruction
   
  relocation_cerr << "  Generating a PC-relative data access (" << insn_.format()
		  << "," << std::hex << addr_ 
		  <<"," << target_ << std::dec << ")" << endl;

  RelDataPatch *newPatch = new RelDataPatch(insn_, target_, addr_);
  newPatch->setBlock(t->block());
  newPatch->setFunc(t->func());
  buffer.addPatch(newPatch, tracker(t));

  return true;
}

string RelDataWidget::format() const {
  stringstream ret;
  ret << "PCRel(" << insn_.format() << ")";
  return ret.str();
}

bool RelDataPatch::apply(codeGen &gen, CodeBuffer *) {
  instruction ugly_insn(orig_insn.ptr(), (gen.width() == 8));
  instPoint *point = gen.point();
  if (!point || (point->type() != instPoint::PreInsn && point->insnAddr() != orig)) {
      point = instPoint::preInsn(func, block, orig, orig_insn, true);
  }
  registerSpace *rs = registerSpace::actualRegSpace(point);
  gen.setRegisterSpace(rs);

  if (!insnCodeGen::modifyData(target_addr, ugly_insn, gen)) {
      relocation_cerr << "RelDataPatch returned false from modifyData (original address: " << std::hex<< orig << ")" <<endl;
      return false;
  }
  return true;
}

unsigned RelDataPatch::estimate(codeGen &) {
   // Underestimate if possible
   return orig_insn.size();
}
