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

////////////////////////
PCWidget::Ptr PCWidget::create(Instruction insn,
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

string PCWidget::format() const {
  stringstream ret;
  ret << "PCWidget(" 
      << std::hex << addr_ << std::dec;
  ret << "" << a_.format();
  return ret.str();
}


unsigned IPPatch::estimate(codeGen &) {
   // should be the minimum required since we expand
   // but never contract

   // In the process case we always just generate it 
   // straight out, because we know the original address.

   // It's gonna be a big one...
   return 1+4+1+1+1+4 + ((type == Reg) ? 1 : 0);
}

