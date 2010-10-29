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
#include "CodeTracker.h"

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;
using namespace Relocation;

CodeMover::Ptr CodeMover::create(CodeTracker &t) {
   // Make a CodeMover
   Ptr ret = Ptr(new CodeMover(t));
   if (!ret) 
      return Ptr();

   return ret;
}  

bool CodeMover::addFunctions(FuncSet::const_iterator begin, 
			     FuncSet::const_iterator end) {
   // A vector of Functions is just an extended vector of basic blocks...
   for (; begin != end; ++begin) {
      int_function *func = *begin;

      if (!func->isInstrumentable()) {
         cerr << "Skipping func " << func->symTabName() << " that's uninstrumentable" << endl;
         continue;
      }

      if (!addTraces(func->blocks().begin(), func->blocks().end())) {
         return false;
      }
    
      // Add the function entry as Required in the priority map
      bblInstance *entry = func->entryBlock()->origInstance();
      priorityMap_[entry] = Required;
   }

   return true;
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

      priorityMap_[bbl] = Suggested;
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

bool CodeMover::initialize(const codeGen &templ) {
   buffer_.initialize(templ);

   // Tell all the blocks to do their generation thang...
   for (TraceList::iterator i = blocks_.begin(); 
        i != blocks_.end(); ++i) {
      // Grab the next block
      TraceList::iterator tmp = i; tmp++;
      Trace::Ptr next;
      if (tmp != blocks_.end()) 
         next = *tmp;
      
      if (!(*i)->generate(templ, buffer_))
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

   if (!buffer_.generate(addr)) return false;
   
   buffer_.extractTrackers(tracker_);

   return true;
}

void CodeMover::disassemble() const {
   buffer_.disassemble();
}

unsigned CodeMover::size() const {
   return buffer_.size();
}

void *CodeMover::ptr() const {
   return buffer_.ptr();
}

codeGen &CodeMover::gen() {
   return buffer_.gen();
}

///////////////////////

PriorityMap &CodeMover::priorityMap() {
   return priorityMap_;
}

///////////////////////

SpringboardMap &CodeMover::sBoardMap(AddressSpace *as) {
   // Take the current PriorityMap, digest it,
   // and return a sorted list of where we need 
   // patches (from and to)

   relocation_cerr << "Creating springboard request map" << endl;

   if (sboardMap_.empty()) {
      for (PriorityMap::const_iterator iter = priorityMap_.begin();
           iter != priorityMap_.end(); ++iter) {
         bblInstance *bbl = iter->first;
         const Priority &p = iter->second;

         // the priority map may include things not in the block
         // map...
         TraceMap::const_iterator b_iter = blockMap_.find(bbl);
         if (b_iter != blockMap_.end()) {
            TracePtr trace = b_iter->second;
            int labelID = trace->getLabel();
            Address to = buffer_.getLabelAddr(labelID);
            
            sboardMap_.addFromOrigCode(bbl->firstInsnAddr(), to, p, bbl);
            relocation_cerr << "Added map " << hex
                            << bbl->firstInsnAddr() << " -> " 
                            << to << ", " << p << dec << endl;
         }
      }

      // And instrumentation that needs updating
      createInstrumentationSpringboards(as);
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

void CodeMover::extractDefensivePads(AddressSpace *AS) {
   for (std::map<bblInstance *, codeGen::Extent>::iterator iter = gen().getDefensivePads().begin();
        iter != gen().getDefensivePads().end(); ++iter) {
      AS->addDefensivePad(iter->first, iter->second.first, iter->second.second);
   }
}

void CodeMover::createInstrumentationSpringboards(AddressSpace *as) {
   return;

  for (std::map<baseTramp *, Address>::iterator iter = gen().getInstrumentation().begin();
        iter != gen().getInstrumentation().end(); ++iter) {
      std::set<Address>::iterator begin, end;
      as->getPreviousInstrumentationInstances(iter->first, begin, end);
      for (; begin != end; ++begin) {
         sboardMap_.addFromRelocatedCode(*begin, iter->second, RelocSuggested);
         relocation_cerr << "\t Added inst SB " << hex
                         << *begin << " -> " << iter->second << dec << endl;
      }
      as->addInstrumentationInstance(iter->first, iter->second);
   }
   for (std::map<baseTramp *, Address>::iterator iter = gen().getRemovedInstrumentation().begin();
        iter != gen().getRemovedInstrumentation().end(); ++iter) {
      // As above, without the add
      std::set<Address>::iterator begin, end;
      as->getPreviousInstrumentationInstances(iter->first, begin, end);
      for (; begin != end; ++begin) {
         sboardMap_.addFromRelocatedCode(*begin, iter->second, RelocSuggested);
         relocation_cerr << "\t Added inst SB " << hex
                         << *begin << " -> " << iter->second << dec << endl;
      }
   }
}
