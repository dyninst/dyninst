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

Trace::Ptr Trace::create(bblInstance *bbl) {
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

  Ptr newTrace = Ptr(new Trace(inst));

  return newTrace;
}
  
Trace::Ptr Trace::create(Atom::Ptr a) {
  if (!a) return Ptr();
  Ptr newTrace = Ptr(new Trace(a));
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

bool Trace::generate(codeGen &templ,
		     unsigned &sizeEstimate) {
  gens_.initialize(templ);

  relocation_cerr << "Generating block " << id() << " orig @ " << hex << origAddr() << dec << endl;

  // Simple form: iterate over every Atom, in order, and generate it.
  for (AtomList::iterator iter = elements_.begin(); iter != elements_.end(); ++iter) {

    unsigned presize = gens_().used();

    TrackerElement *e = (*iter)->tracker();
    if (e) {
      gens_.cur().trackers[presize] = e;
    }

    if (!(*iter)->generate(gens_)) {
      return false;
      // This leaves the block in an inconsistent state and should only be used
      // for fatal failures.
    }
  }
  
  size_ = gens_.size();
  relocation_cerr << "... block done, size " << size_ << endl;

  sizeEstimate += size_;

  return true;
}

// Go through the list of patches and apply each patch to the appropriate
// codeGen object in our GenStack. Check to see if the total size of said
// has increased; if so, we'll need to regenerate at some point (and
// allocate more room). 

bool Trace::applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift) {
  relocation_cerr << "Trace " << hex << origAddr() << ", now @" 
		  << curAddr_ << " and moving to " << gen.currAddr() << ", shift " << shift << dec << endl;

  iteration_++;

  curAddr_ = gen.currAddr();

  unsigned size = 0;

  for (GenStack::iterator i = gens_.begin(); i != gens_.end(); ++i) {
    i->offset = totalSize + size;
    if (!i->apply(gen, size, iteration_, shift)) {
      return false;
    }
    relocation_cerr << "\t After patch: " << gen.currAddr() - curAddr_ << " bytes used" << endl;
  }

  relocation_cerr << "\t Old size " << size_ << " new size " << size << endl;

  if (size < size_) {
    // Oops...
    relocation_cerr << "WARNING: padding /w/ noops @ " << hex << curAddr_ << " from " << origAddr() << dec << endl;
    gen.fill(size_ - size, codeGen::cgNOP);
  }
  else if (size > size_) {
    shift += (size - size_);
    regenerate = true;
    size_ = size;
  }

  totalSize += size_;
  
  return true;
}  

bool Trace::extractTrackers(CodeTracker &t) {
  // Each GenStack element has a set of point pairs <curOffset, TrackerElement *>
  // where curOffset is some magic pointer into the GenStack structure
  // that is a) unique and b) can be converted to an absolute address.
  //
  // We need to update it to make up for patches and get the size field right.

  if (gens_.begin() == gens_.end()) return true;
  // There's a math problem here; the offset from the first generator in the stack
  // is also included in the curAddr_ address we're handed. So add in a correcting
  // factor
  unsigned corrective_factor = gens_.begin()->offset;

  for (GenStack::iterator gen = gens_.begin(); gen != gens_.end(); ++gen) {
    for (GenStack::Trackers::iterator tmap = gen->trackers.begin();
	 tmap != gen->trackers.end(); ++tmap) {
      
      TrackerElement *e = tmap->second;
      assert(e);

      unsigned size = 0;
      // Work out the size
      GenStack::Trackers::iterator next = tmap;
      ++next;
      if (next != gen->trackers.end()) {
	size = next->first - tmap->first;
      }
      else {
	size = (*gen).gen.used() - tmap->first;
      }
      if (!size) continue;

      Address relocAddr = tmap->first + gen->offset + curAddr_ - corrective_factor;
      
      assert(e);

      e->setReloc(relocAddr);
      e->setSize(size);
      
      t.addTracker(e);
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

/////////////////////////

GenStack::GenObj::~GenObj() {
  if (patch) delete patch;
}


bool GenStack::GenObj::apply(codeGen &current, unsigned &size, int iteration, int shift) {
  gen.setAddr(current.currAddr());

  relocation_cerr << "\t\t Setting local gen to addr " 
		  << hex << current.currAddr() << dec << endl;

  // Generate our patch
  if (patch) {
    gen.setIndex(index);
    relocation_cerr << "\t\t Using patch: setting gen to index " << index << " and " 
	 << gen.used() << " bytes used" << endl;
    if (!patch->apply(gen, iteration, shift)) return false;
  }

  
  // And copy it into the master codeGen
  current.copy(gen);
  size += gen.used();

  relocation_cerr << "\t\t " << gen.used() << " bytes used" << endl;

  return true;
}

void GenStack::initialize(codeGen &gen) {
  assert(gen.codeEmitter());
  gens_.clear();
  gens_.push_back(GenObj());
  cur().gen.applyTemplate(gen);
  cur().gen.allocate(16);
}

void GenStack::inc() {
  assert(!gens_.empty());
  GenObj &old = gens_.back();
  gens_.push_back(GenObj());
  cur().gen.applyTemplate(old.gen);
  assert(cur().gen.codeEmitter());
  cur().gen.allocate(16);
}

unsigned GenStack::size() {
  unsigned ret = 0;
  for (iterator i = begin(); i != end(); ++i) {
    ret += i->gen.used();
  }
  return ret;
}

void GenStack::addPatch(Patch *patch) {
  // Tack the given patch onto the end
  // of the current GenObj, then roll the stack over
  assert(gens_.back().patch == NULL);

  gens_.back().patch = patch;
  gens_.back().index = cur().gen.getIndex();
  
  relocation_cerr << "\t\t Adding patch to codeGen stack; cur at " << gens_.back().gen.used() 
		  << " in stack " << gens_.back().ptr() << endl;

  patch->preapply(gens_.back().gen);
  relocation_cerr << " and after preapply " << gens_.back().gen.used() << endl;

  inc();
}


