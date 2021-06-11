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

#include "Relocation.h"
#include "CodeMover.h"
#include "Widgets/Widget.h"
#include "CFG/RelocBlock.h"

#include "instructionAPI/h/InstructionDecoder.h" // for debug

#include "dyninstAPI/src/addressSpace.h" // Also for debug
#include "dyninstAPI/src/function.h"

#include "dyninstAPI/src/debug.h"
#include "CodeTracker.h"
#include "CFG/RelocGraph.h"

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;
using namespace Relocation;

CodeMover::CodeMover(CodeTracker *t) :
   cfg_(new RelocGraph()),
   addr_(0),
   tracker_(t),
   finalized_(false) {}

CodeMover::Ptr CodeMover::create(CodeTracker *t) {

   // Make a CodeMover
   Ptr ret = Ptr(new CodeMover(t));
   if (!ret) 
      return Ptr();

   return ret;
}  

CodeMover::~CodeMover() {
   delete cfg_;
   // Do not delete codeTracker
}

bool CodeMover::addFunctions(FuncSet::const_iterator begin, 
			     FuncSet::const_iterator end) {
   // A vector of Functions is just an extended vector of basic blocks...
   for (; begin != end; ++begin) {
      func_instance *func = *begin;
      if (!func->isInstrumentable()) {
	relocation_cerr << "\tFunction " << func->symTabName() << " is non-instrumentable, skipping" << endl;
         continue;
      }
      relocation_cerr << "\tAdding function " << func->symTabName() << endl;
      //if (!addRelocBlocks(func->blocks().begin(), func->blocks().end(), func)) {
      if (!addRelocBlocks(func->blocks().begin(), func->blocks().end(), func)) {
         return false;
      }
    
      // Add the function entry as FuncEntry in the priority map
      block_instance *entry = func->entryBlock();
      priorityMap_[std::make_pair(entry, func)] = FuncEntry;
      relocation_cerr << "\t Added FuncEntry for " << func->symTabName() << " / " << hex << entry->start() << dec << endl;
   }

   return true;
}

template <typename RelocBlockIter>
bool CodeMover::addRelocBlocks(RelocBlockIter begin, RelocBlockIter end, func_instance *f) {
   for (; begin != end; ++begin) {
     addRelocBlock(SCAST_BI(*begin), f);
   }
   return true;
}

bool CodeMover::addRelocBlock(block_instance *bbl, func_instance *f) {
   RelocBlock * block = RelocBlock::createReloc(bbl, f);
   if (!block)
      return false;
   cfg_->addRelocBlock(block);
   
   if (!bbl->wasUserAdded()) {
     relocation_cerr << "\t Added suggested entry for " << f->symTabName() << " / " << hex << bbl->start() << dec << endl;
     priorityMap_[std::make_pair(bbl, f)] = Suggested;
   }

   return true;
}

void CodeMover::finalizeRelocBlocks() {
   if (finalized_) return;

   finalized_ = true;
   for (RelocBlock *iter = cfg_->begin(); iter != cfg_->end(); iter = iter->next()) {
      iter->linkRelocBlocks(cfg_);
      iter->determineSpringboards(priorityMap_);
   }
}
   


///////////////////////


bool CodeMover::transform(Transformer &t) {
   if (!finalized_)
      finalizeRelocBlocks();

   bool ret = true; 

   t.processGraph(cfg_);

   return ret;
}

bool CodeMover::initialize(const codeGen &templ) {
   buffer_.initialize(templ, cfg_->size);

   // If they never called transform() this can get missed.
   if (!finalized_)
      finalizeRelocBlocks();
   
   // Tell all the blocks to do their generation thang...
   for (RelocBlock *iter = cfg_->begin(); iter != cfg_->end(); iter = iter->next()) {
      if (!iter->finalizeCF()) return false;
      
      if (!iter->generate(templ, buffer_)) {
         cerr << "ERROR: failed to generate RelocBlock!" << endl;
         return false; // Catastrophic failure
      }
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
   return true;
}

bool CodeMover::finalize() {     
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

SpringboardMap &CodeMover::sBoardMap(AddressSpace *) {
   // Take the current PriorityMap, digest it,
   // and return a sorted list of where we need 
   // patches (from and to)

   relocation_cerr << "Creating springboard request map" << endl;

   if (sboardMap_.empty()) {
      for (PriorityMap::const_iterator iter = priorityMap_.begin();
           iter != priorityMap_.end(); ++iter) {
         block_instance *bbl = iter->first.first;
         const Priority &p = iter->second;
         func_instance *func = iter->first.second;

	 relocation_cerr << "Func " << func->symTabName() << " / block " 
			 << hex << bbl->start() << " /w/ priority " << p 
			 << dec << endl;

         if (bbl->wasUserAdded()) continue;

         // the priority map may include things not in the block
         // map...
         RelocBlock * trace = cfg_->findSpringboard(bbl, func);
         if (!trace) continue;
         int labelID = trace->getLabel();
         Address to = buffer_.getLabelAddr(labelID);
         if (bbl->_ignorePowerPreamble) { 
             relocation_cerr << "\t" << hex << "springboard target " << bbl->start() + 0x8 << endl;
             sboardMap_.addFromOrigCode(bbl->start() + 0x8, to, p, func, bbl);
         }
         else {
             relocation_cerr << "\t" << hex << "springboard target " << bbl->start() << endl;
             sboardMap_.addFromOrigCode(bbl->start(), to, p, func, bbl);
         }
      }
      
      // And instrumentation that needs updating
      //createInstrumentationSpringboards(as);
   }

   return sboardMap_;
}

string CodeMover::format() const {
   stringstream ret;
  
   ret << "CodeMover() {" << endl;

   for (RelocBlock *iter = cfg_->begin(); iter != cfg_->end(); iter = iter->next()) {
      ret << iter->format();
   }
   ret << "}" << endl;
   return ret.str();

}

void CodeMover::extractDefensivePads(AddressSpace *AS) {
   // For now, we're doing an annoying iteration over all CodeTracker elements looking
   // for any padding structures. TODO: roll this into the address lookup
   // mechanism.
   const CodeTracker::TrackerList &trackers = tracker_->trackers();
   for (CodeTracker::TrackerList::const_iterator iter = trackers.begin(); iter != trackers.end(); ++iter) {
      if ((*iter)->type() == TrackerElement::padding) {
         PaddingTracker *tmp = static_cast<PaddingTracker *>(*iter);
         AS->addDefensivePad(tmp->block(), tmp->func(), tmp->reloc(), tmp->pad());
      }
   }
}

