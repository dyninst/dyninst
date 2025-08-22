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

#include "StackModWidget.h"
#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/debug.h"
#include "CFG.h"
#include "../CFG/RelocBlock.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

StackModWidget::Ptr StackModWidget::create(Instruction insn,
                                           Address addr,
                                           signed long newDisp,
                                           Architecture arch) {
  assert(addr);
  return Ptr(new StackModWidget(insn, addr, newDisp, arch));
}

TrackerElement *StackModWidget::tracker(const RelocBlock *t) const {
   EmulatorTracker *e = new EmulatorTracker(addr_, t->block(), t->func());
  return e;
}

bool StackModWidget::generate(const codeGen &, 
                              const RelocBlock *t, 
                              CodeBuffer &buffer) {
    relocation_cerr << "  Generating a stackframe-sensitive memory access (" << insn_.format()
        << "," << std::hex << addr_ 
        <<", newDisp " << newDisp_ << std::dec << ")" << endl;

    StackModPatch *newPatch = new StackModPatch(insn_, newDisp_, arch_, addr_);
    buffer.addPatch(newPatch, tracker(t));

  return true;
}

string StackModWidget::format() const {
  stringstream ret;
  ret << "MemRel(" << insn_.format() << ")";
  return ret.str();
}

bool StackModPatch::apply(codeGen &gen, CodeBuffer *) {
#if defined(cap_stack_mods)
    instruction ugly_insn(orig_insn.ptr(), (gen.width() == 8));
    if (gen.modifiedStackFrame()) {
        relocation_cerr << "  Calling modifyDisp" << endl;
        if (!insnCodeGen::modifyDisp(newDisp, ugly_insn, gen, arch, addr)) 
            return false;
        return true;
    } else {
        relocation_cerr << "  Preserving orig" << endl;
        // Preserve the original instruction
        GET_PTR(newInsn, gen);
        const unsigned char* origInsn = ugly_insn.ptr();
        for (unsigned iter = 0; iter < ugly_insn.size(); iter++) {
            *newInsn++ = *origInsn++;
        }
        SET_PTR(newInsn, gen);
    }
    return true;
#else
    (void)gen; // Silence compiler warning
    return false;
#endif
}

unsigned StackModPatch::estimate(codeGen &) {
   // Underestimate if possible
   return orig_insn.size();
}
