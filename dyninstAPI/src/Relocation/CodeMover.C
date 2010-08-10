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

#include "Relocation.h"
#include "CodeMover.h"
#include "Atoms/Atom.h"

#include "dyninstAPI/src/function.h" // bblInstance, int_basicTrace, int_function...

#include "instructionAPI/h/InstructionDecoder.h" // for debug
#include "dyninstAPI/src/addressSpace.h" // Also for debug

#include "dyninstAPI/src/debug.h"

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;
using namespace Relocation;

CodeMover::Ptr CodeMover::create() {
  // Make a CodeMover
  Ptr ret = Ptr(new CodeMover());
  if (!ret) 
    return Ptr();

  return ret;
}  

template <typename TraceIter>
CodeMover::Ptr CodeMover::create(TraceIter begin,
				 TraceIter end) {
  Ptr ret = create();
  if (!ret) return Ptr();
  
  // Now for the hard part. We need to turn the vector of
  // int_basicTraces into a vector of Relocation::Traces.  Those take
  // bblInstances as inputs. I'm going to assume no multiple
  // relocation, and so we can just use the primary bblInstance for
  // each int_basicTrace.
  
  if (!ret->addTraces(begin, end))
    return Ptr();
  
  return ret;
};


CodeMover::Ptr CodeMover::createFunc(FuncSet::const_iterator begin, FuncSet::const_iterator end) {
  Ptr ret = create();
  if (!ret) return Ptr();

  static int count = 0;

  // A vector of Functions is just an extended vector of basic blocks...
  for (; begin != end; ++begin) {
    int_function *func = *begin;
    relocation_cerr << "Adding function " << func->symTabName() << endl;

    count++;

    if (!func->isInstrumentable()) {
      cerr << "Skipping func " << func->symTabName() << " that's uninstrumentable" << endl;
      continue;
    }

    if (!ret->addTraces(func->blocks().begin(), func->blocks().end())) {
      return Ptr();
    }

    // Add the function entry as Required in the priority map
    bblInstance *entry = func->entryBlock()->origInstance();
    ret->priorityMap_[entry] = Required;
  }


  //cerr << "Count is " << count << endl;
  return ret;
}

template <typename TraceIter>
bool CodeMover::addTraces(TraceIter begin, TraceIter end) {
  for (; begin != end; ++begin) {
    bblInstance *bbl = (*begin)->origInstance();
    relocation_cerr << "Creating Trace for bbl at " 
		    << std::hex << bbl->firstInsnAddr() << std::dec
		    << endl;
    Trace::Ptr block = Trace::create(bbl);
    if (!block)
      return false;
    blocks_.push_back(block);
    blockMap_[bbl] = block;
    relocation_cerr << "  Updated block map: " 
		    << std::hex << bbl->firstInsnAddr() << std::dec
		    << "->" << block.get() << endl;

  }
  return true;
}

bool CodeMover::addTrace(bblInstance *bbl) {
  relocation_cerr << "Creating Trace for bbl at " 
		  << std::hex << bbl->firstInsnAddr() << std::dec
		  << endl;
  Trace::Ptr block = Trace::create(bbl);
  if (!block)
    return false;
  blocks_.push_back(block);
  blockMap_[bbl] = block;
  relocation_cerr << "  Updated block map: " 
		  << std::hex << bbl->firstInsnAddr() << std::dec
		  << "->" << block.get() << endl;
  
  return true;
}


///////////////////////


bool CodeMover::transform(Transformer &t) {
  bool ret = true; 

  t.preprocess(blocks_);
  for (TraceList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    if (!t.processTrace(i))
      ret = false;
  }
  t.postprocess(blocks_);

  return ret;
}

bool CodeMover::initialize(const codeGen &t) {
  // Accumulate initial size guess
  // We'll expand this as necessary...
  size_ = 0;

  gen_.applyTemplate(t);

  // Tell all the blocks to do their generation thang...
  for (TraceList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    // Grab the next block
    TraceList::iterator tmp = i; tmp++;
    Trace::Ptr next;
    if (tmp != blocks_.end()) 
      next = *tmp;

    if (!(*i)->generate(gen_, size_))
      return false; // Catastrophic failure
  }
  return true;
}

// And now the fun begins
// 
// We wish to minimize the space required by the relocated code. Since some platforms
// may have varying space requirements for certain instructions (e.g., branches) this
// requires a fixpoint calculation. We start with the original size and increase from
// there. 
// 
// Reasons for size increase:
//   1) Instrumentation. It tends to use room. Odd.
//   2) Transformed instructions. We may need to replace a single instruction with a
//      sequence to emulate its original behavior
//   3) Variable-sized instructions. If we increase branch displacements we may need
//      to increase the corresponding branch instruction sizes.

bool CodeMover::relocate(Address addr) {
  addr_ = addr;
  gen_.setAddr(addr);

  int globalChanged = 0;
  int pass = 1;

  // Priming pass; assuming each block has its current size
  Address tmpAddr = addr;

  for (TraceList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    (*i)->setAddr(tmpAddr);
    tmpAddr += (*i)->size();
    (*i)->resetIteration();
  }

  do {
    // Reset the flag...
    globalChanged = 0;

    gen_.allocate(size_);
    
    unsigned newSize = 0;

    int shift = 0;

    for (TraceList::iterator i = blocks_.begin(); 
	 i != blocks_.end(); ++i) {
      bool blockChanged = false;
      if (!(*i)->applyPatches(gen_, blockChanged, newSize, shift))
	return false; // Catastrophic failure
      if (blockChanged) {
	globalChanged++;
      }
      relocation_cerr << "    ... post patching, local " 
		      << blockChanged << " and global " 
		      << globalChanged << endl;
    }
    
    if (newSize > size_) {
      // Should have been caught in the blocks, so assert...
      assert(globalChanged > 0);
      size_ = newSize;
    }

    pass++;
  } 
  while (globalChanged);

  // Update the entry map
  for (TraceList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    entryMap_[(*i)->origAddr()] = (*i)->curAddr();
  }

  return true;
}

void CodeMover::extractAddressMap(AddressMapper &addrMap, Address baseAddr) {

  AddressMapper::accumulatorList accumulators;

  for (TraceList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    (*i)->extractAddressMap(accumulators);
  }

  addrMap.createTrees(accumulators, baseAddr);
}


void CodeMover::disassemble() const {
  // InstructionAPI to the rescue!!!
  InstructionAPI::InstructionDecoder decoder(gen_.start_ptr(),
				       gen_.used(),
				       gen_.getArch());
  Address addr = gen_.startAddr();

  Instruction::Ptr cur = decoder.decode();
  while (cur && cur->isValid()) {
    cerr << std::hex << addr << std::dec << " " << cur->format() << endl;
    addr += cur->size();
    cur = decoder.decode();
  }
}

unsigned CodeMover::size() const {
  if (gen_.used()) return gen_.used();

  // Try and get an estimate of the block sizes.... ugh.
  unsigned size = 0;

  for (TraceList::const_iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    size += (*i)->estimateSize();
  }
  return size;
}

///////////////////////

PriorityMap &CodeMover::priorityMap() {
  return priorityMap_;
}

///////////////////////

const SpringboardMap &CodeMover::sBoardMap() {
  // Take the current PriorityMap, digest it,
  // and return a sorted list of where we need 
  // patches (from and to)

  if (sboardMap_.empty()) {
    for (PriorityMap::const_iterator iter = priorityMap_.begin();
	 iter != priorityMap_.end(); ++iter) {
      bblInstance *from = iter->first;
      const Priority &p = iter->second;

      // the priority map may include things not in the block
      // map...
      TraceMap::const_iterator b_iter = blockMap_.find(from);
      if (b_iter != blockMap_.end()) {
	const Address &to = b_iter->second->curAddr();
	sboardMap_.add(from->firstInsnAddr(), to, p);
      }
    }
  }

  return sboardMap_;
}

string CodeMover::format() const {
  stringstream ret;
  
  ret << "CodeMover() {" << endl;

  for (TraceList::const_iterator iter = blocks_.begin();
       iter != blocks_.end(); ++iter) {
    ret << (*iter)->format();
  }
  ret << "}" << endl;
  return ret.str();

}

void CodeMover::extractPostCallPads(AddressSpace *AS) {
  for (std::set<std::pair<Address, Address> >::iterator iter = gen_.getPostCallPads().begin();
       iter != gen_.getPostCallPads().end(); ++iter) {
    AS->addPostCallPad(iter->first, iter->second);
  }
}
