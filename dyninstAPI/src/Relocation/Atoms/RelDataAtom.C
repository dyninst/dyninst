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

#include "RelDataAtom.h"
#include "instructionAPI/h/Instruction.h"
#include "../patchapi_debug.h"
#include "CFG.h"
#include "Trace.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

RelDataAtom::Ptr RelDataAtom::create(Instruction::Ptr insn,
					   Address addr,
					   Address target) {
  assert(addr);
  return Ptr(new RelDataAtom(insn, addr, target));
}

TrackerElement *RelDataAtom::tracker(int_block *b) const {
   EmulatorTracker *e = new EmulatorTracker(addr_, b);
  return e;
}

bool RelDataAtom::generate(const codeGen &, 
                              const Trace *t, 
                              CodeBuffer &buffer) {
  // We want to take the original instruction and emulate
  // it at whatever our new address is. 

  // Fortunately, we can reuse old code to handle the
  // translation

  // Find the original target of the instruction 
   
  relocation_cerr << "  Generating a PC-relative data access (" << insn_->format()
		  << "," << std::hex << addr_ 
		  <<"," << target_ << std::dec << ")" << endl;

  RelDataPatch *newPatch = new RelDataPatch(insn_, target_);
  buffer.addPatch(newPatch, tracker(t->block()));

  return true;
}

string RelDataAtom::format() const {
  stringstream ret;
  ret << "PCRel(" << insn_->format() << ")";
  return ret.str();
}

bool RelDataPatch::apply(codeGen &gen, CodeBuffer *) {
  instruction ugly_insn(orig_insn->ptr());
  pcRelData pcr(target_addr, ugly_insn);
  pcr.gen = &gen;
  pcr.apply(gen.currAddr());
  return true;
}

unsigned RelDataPatch::estimate(codeGen &) {
   // Underestimate if possible
   return orig_insn->size();
}
