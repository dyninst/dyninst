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

#include "r_Relocation.h"
#include "r_t_Base.h" // transformer class
#include "r_e_Base.h"
#include "r_e_CopyInsn.h" // Default Element in each Block
#include "r_e_Instrumentation.h" // For instrumentation Blocks
#include "debug.h"

#include <sstream>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Relocation;
using namespace std;

// Let's do the block class first. 

// A Block is a container for a set of instructions and
// instrumentation that we generate out as a single unit. To make life
// easier, it's currently matching the (implied) single-control-flow
// path assumption. That means that edge instrumentation must go into
// a separate Block. Darn.
// 
// A Block consists of three logical types of elements: instructions,
// instrumentation, and flow controllers.
//
// Instruction: a reference to an original code instruction that is
// being moved as part of this block.

// Instrumentation: a reference to a (currently) BaseTrampInstance
// that contains an entire, single-entry, single-exit sequence of
// instrumentation.

// Flow controllers: an abstraction of control flow instructions
// (branches/calls) that end basic blocks. One effect of moving code
// around is that control flow gets b0rked pretty badly, so we toss
// the original instruction and regenerate it later.

int Block::BlockID = 0;

Block::Ptr Block::create(bblInstance *bbl) {
  if (!bbl) return Block::Ptr();

  relocation_cerr << "Creating new Block" << endl;


  Ptr newBlock = Ptr(new Block(bbl));  

  // Get the list of instructions in the block
  std::vector<std::pair<Instruction::Ptr, Address> > insns;
  bbl->getInsnInstances(insns);

  for (unsigned i = 0; i < insns.size(); ++i) {
    relocation_cerr << "  Adding instruction " 
		    << std::hex << insns[i].second << std::dec
		    << " " << insns[i].first->format() << endl;
    Element::Ptr ptr = CopyInsn::create(insns[i].first,
					 insns[i].second);
    if (!ptr) {
      // And this will clean up all of the created elements. Nice. 
      return Ptr();
    }
    
    newBlock->elements_.push_back(ptr);
  }

  return newBlock;
}

Block::Ptr Block::create(baseTramp *base) {
  // Instrumentation-only block... must be for 
  // an edge!

  if (!base) return Ptr();

  // Create an Inst element
  Inst::Ptr inst = Inst::create();
  if (!inst) return Ptr();
  inst->addBaseTramp(base);

  Ptr newBlock = Ptr(new Block(base));

  newBlock->elements_.push_back(inst);
  return newBlock;
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

bool Block::generate(codeGen &templ,
		     unsigned &sizeEstimate) {
  gens_.initialize(templ);

  relocation_cerr << "Generating block " << id() << " orig @ " << hex << origAddr() << dec << endl;

  Address last = 0;

  // Simple form: iterate over every Element, in order, and generate it.
  for (ElementList::iterator iter = elements_.begin(); iter != elements_.end(); ++iter) {
    if ((*iter)->addr() &&
	(*iter)->size()) {
      last = (*iter)->addr() + (*iter)->size();
    }  

    gens_.cur().addrMap.push_back(std::make_pair<Address, Offset>((*iter)->addr(),
								  gens_().used()));
    
    if (!(*iter)->generate(*this, gens_)) {
      cerr << "Error: failed to generate element " << (*iter)->format() << endl;
      return false;
      // This leaves the block in an inconsistent state and should only be used
      // for fatal failures.
    }
  }
  
  if (last) {
    gens_.cur().addrMap.push_back(std::make_pair<Address, Offset>(last,
								  gens_().used()));
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

bool Block::applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift) {
  relocation_cerr << "Block " << hex << origAddr() << ", now @" 
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

bool Block::extractAddressMap(AddressMapper::accumulatorList &accumulators) {
  // Each GenStack element has a set of point pairs <origAddr, relocAddr>
  // For a given n, 0 <= n < max, we want to create an accumulator that is the range
  // between adjacent points; that is
  // (n.first, n.second, (n+1).first - n.first, (n+1).second - n.second)
  // where the 2nd point also has the accumulated Offset from the stack element added in.

  Address lastAddr = 0;
  Offset lastOffset = (Offset) -1;
  Address nextAddr = lastAddr;
  Offset nextOffset = lastOffset;
  
  for (GenStack::iterator i = gens_.begin(); i != gens_.end(); ++i) {
    for (std::list<std::pair<Address, Offset> >::iterator j = i->addrMap.begin();
	 j != i->addrMap.end(); ++j) {

      // Initialize lastOffset to the first thing we see
      if (nextOffset == ((Offset) -1)) {
	nextOffset = j->second + i->offset;
      }

      if (j->first) {
	nextAddr = j->first;
	if (lastAddr) {
	  nextOffset = j->second + i->offset;
	  accumulators.push_back(AddressMapper::accumulator(lastAddr,
							    lastOffset,
							    nextAddr - lastAddr,
							    nextOffset - lastOffset));
	}
	lastAddr = nextAddr;
	lastOffset = nextOffset;
      }
    }
  }
  
  return true;
}
			       




std::string Block::format() const {
  stringstream ret;
  ret << "Block(" 
      << std::hex << origAddr() << std::dec
      << "/" << id() 
      << ") {" << endl;
  for (ElementList::const_iterator iter = elements_.begin();
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
  
  relocation_cerr << "\t\t Adding patch to codeGen stack; cur at " << gens_.back().gen.used() << endl;

  patch->preapply(gens_.back().gen);
  relocation_cerr << " and after preapply " << gens_.back().gen.used() << endl;

  inc();
}


