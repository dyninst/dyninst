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


#include <iostream>
#include <iomanip>

#include "../Transformers/Transformer.h" // transformer class
#include "Atom.h"
#include "CopyInsn.h" // Default Atom in each Trace
#include "Instrumentation.h" // For instrumentation Traces
#include "dyninstAPI/src/debug.h"
#include "../CodeTracker.h"
#include <sstream>
#include "Target.h"
#include "../CodeBuffer.h"
#include "dyninstAPI/src/baseTramp.h"


using namespace Dyninst;
using namespace InstructionAPI;
using namespace Relocation;
using namespace std;

// Let's do the block class first. 

// A Trace is a container for a set of instructions and
// instrumentation that we generate out as a single unit. To make life
// easier, it's currently matching the (implied) single-control-flow
// path assumption. That means that edge instrumentation must go into
// a separate Trace. Darn.
// 
// A Trace consists of three logical types of elements: instructions,
// instrumentation, and flow controllers.
//
// Instruction: a reference to an original code instruction that is
// being moved as part of this block.

// Instrumentation: a reference to a (currently) AtomTrampInstance
// that contains an entire, single-entry, single-exit sequence of
// instrumentation.

// Flow controllers: an abstraction of control flow instructions
// (branches/calls) that end basic blocks. One effect of moving code
// around is that control flow gets b0rked pretty badly, so we toss
// the original instruction and regenerate it later.

int Trace::TraceID = 0;

Trace::Ptr Trace::create(int_block *bbl) {
  if (!bbl) return Trace::Ptr();

  relocation_cerr << "Creating new Trace" << endl;

  Ptr newTrace = Ptr(new Trace(bbl));  

  // Get the list of instructions in the block
  std::vector<std::pair<Instruction::Ptr, Address> > insns;
  bbl->getInsnInstances(insns);

  for (unsigned i = 0; i < insns.size(); ++i) {
    relocation_cerr << "  Adding instruction " 
		    << std::hex << insns[i].second << std::dec
		    << " " << insns[i].first->format() << endl;
    Atom::Ptr ptr = CopyInsn::create(insns[i].first,
				     insns[i].second);
    if (!ptr) {
      // And this will clean up all of the created elements. Nice. 
      return Ptr();
    }
    
    newTrace->elements_.push_back(ptr);
  }

  return newTrace;
}

Trace::Ptr Trace::create(baseTramp *base) {
  // Instrumentation-only block... must be for 
  // an edge!

  if (!base) return Ptr();

  // Create an Inst element
  Inst::Ptr inst = Inst::create();
  if (!inst) return Ptr();
  inst->addBaseTramp(base);

  Ptr newTrace = Ptr(new Trace(inst, base->instP()->addr(), base->instP()->func()));

  return newTrace;
}
  
Trace::Ptr Trace::create(Atom::Ptr a, Address addr, int_function *f) {
  if (!a) return Ptr();
  Ptr newTrace = Ptr(new Trace(a, addr, f));
  return newTrace;
}

// Returns false only on catastrophic failure.

// Returns false on catastrophic failure
// Sets changed to true if something about this block
// changed in this generation; e.g. if we should re-run the
// fixpoint algorithm.
//
// Our fixpoint is to minimize the size of the generated code w.r.t.
// branches. So we "change" if the address of the first instruction changes
// (since it might be a target) or if the size of the block changes...
//
// So basically if the incoming start address is different or if the
// block size changed.

bool debug_blocks = false;

bool disassemble_reloc = false;

bool Trace::generate(const codeGen &templ,
                     CodeBuffer &buffer) {
   relocation_cerr << "Generating block " << id() << " orig @ " << hex << origAddr() << dec << endl;
   
   // Register ourselves with the CodeBuffer and get a label
   label_ = buffer.getLabel();

   static bool debug = false;
   bool insn_debug = false;
#if 1
   if (bbl() && bbl()->start() >= 0xc00000 && bbl()->start() <= 0x1000000) {
       debug = true;
       disassemble_reloc = true;
   }
   else {
       debug = false;
   }
#endif
   Address lastDebugAddr = 0;

   // Simple form: iterate over every Atom, in order, and generate it.
   for (AtomList::iterator iter = elements_.begin(); iter != elements_.end(); ++iter) {
       
       if (bbl() && debug && ((iter == elements_.begin()) || insn_debug)) {
          Address addr = (*iter)->addr();
          if (addr && addr != lastDebugAddr) 
          {
              // We need a DebugTracker type. I'm using InstTracker, so _DO NOT_
              // add instrumentation in the middle of an emulation sequence...
              codeGen gen(1);
              gen.fill(1, codeGen::cgTrap);
              buffer.addPIC(gen, new InstTracker(addr, NULL, bbl()));
              lastDebugAddr = addr;
          }
      }
      if (!(*iter)->generate(templ, 
          this,
          buffer)) 
      {
          return false;
          // This leaves the block in an inconsistent state and should only be used
          // for fatal failures.
      }
   }
   
   return true;
}

std::string Trace::format() const {
    stringstream ret;
  ret << "Trace(" 
      << std::hex << origAddr() << std::dec
      << "/" << id() 
      << ") {" << endl;
  for (AtomList::const_iterator iter = elements_.begin();
       iter != elements_.end(); ++iter) {
    ret << "  " << (*iter)->format() << endl;
  }
  ret << "}" << endl;
  return ret.str();
}

int Target<int_block *>::label(CodeBuffer *buf) const {
   return buf->defineLabel(t_->start());
}

int Target<Address>::label(CodeBuffer *buf) const {
   return buf->defineLabel(t_);
}
   
