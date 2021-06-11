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

#include "CodeTracker.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/block.h"
#include "dyninstAPI/src/addressSpace.h"

#include <iostream>

using namespace Dyninst;
using namespace Relocation;
using namespace std;

CodeTracker::CodeTracker() {
}

CodeTracker::~CodeTracker() {
   // Pile of deallocatable stuff
   for (TrackerList::iterator iter = trackers_.begin();
        iter != trackers_.end(); ++iter) {
      delete (*iter);
   }
   trackers_.clear();
}

CodeTracker *CodeTracker::fork(CodeTracker *parent, 
                              AddressSpace *child) {
   // Duplicate our tracking data structures.
   CodeTracker *newCT = new CodeTracker();
   for (TrackerList::iterator iter = parent->trackers_.begin();
        iter != parent->trackers_.end(); ++iter) {
      TrackerElement *pE = *iter;
      TrackerElement *cE = NULL;
      block_instance *cB = child->findBlock(pE->block()->llb());
      func_instance *cF = (pE->func() ? child->findFunction(pE->func()->ifunc()) : NULL);
      switch (pE->type()) {
         case TrackerElement::original:
            cE = new OriginalTracker(pE->orig(), cB, cF);
            break;
         case TrackerElement::emulated:
            cE = new EmulatorTracker(pE->orig(), cB, cF);
            break;
         case TrackerElement::instrumentation: {
            InstTracker *pI = static_cast<InstTracker *>(pE);
            baseTramp *bt = baseTramp::fork(pI->baseT(), child);
            cE = new InstTracker(pE->orig(), bt, cB, cF);
            break;
         }
         case TrackerElement::padding: {
            PaddingTracker *pP = static_cast<PaddingTracker *>(pE);
            cE = new PaddingTracker(pE->orig(), pP->pad(), cB, cF);
            break;
         }
      }
      cE->setReloc(pE->reloc());
      cE->setSize(pE->size());
      newCT->addTracker(cE);
   }
   return newCT;
}


bool CodeTracker::origToReloc(Address origAddr,
                              block_instance *block,
                              func_instance *func,
                              RelocatedElements &reloc) const {
   ForwardMap::const_iterator iter = origToReloc_.find(block->start());
   if (iter == origToReloc_.end()) return false;

   FwdMapMiddle::const_iterator iter2 = iter->second.find((func ? func->addr() : 0));
   if (iter2 == iter->second.end()) return false;

   FwdMapInner::const_iterator iter3 = iter2->second.find(origAddr);
   if (iter3 == iter2->second.end()) return false;
   
   reloc = iter3->second;
   return true;
}

bool CodeTracker::relocToOrig(Address relocAddr, 
                              RelocInfo &ri) const {
  TrackerElement *e = NULL;
  if (!relocToOrig_.find(relocAddr, e))
     return false;
  ri.orig = e->relocToOrig(relocAddr);
  ri.block = e->block();
  ri.func = e->func();
  ri.reloc = relocAddr;
  if (e->type() == TrackerElement::instrumentation) {
     InstTracker *i = static_cast<InstTracker *>(e);
     ri.bt = i->baseT();
  }
  if (e->type() == TrackerElement::padding) {
     PaddingTracker *p = static_cast<PaddingTracker *>(e);
     ri.pad = p->pad(); 
  }

  return true;
}

TrackerElement *CodeTracker::findByReloc(Address addr) const {
  TrackerElement *e = NULL;
  if (!relocToOrig_.find(addr, e))
    return NULL;
  return e;
}

void CodeTracker::addTracker(TrackerElement *e) {

  // We should look into being more efficient by collapsing ranges
  // of relocated code into a single OriginalTracker

  // If that happens, the assumption origToReloc makes that we can
  // get away without an IntervalTree will be violated and a lot
  // of code will need to be rewritten.
   //relocation_cerr << "Adding tracker: " << *e << endl;

   if (!trackers_.empty()) {
      TrackerElement *previous = trackers_.back();
      if (previous->mergeable() &&
          e->mergeable() && 
          previous->type() == e->type() &&
          e->orig() == (previous->orig()+previous->size()) &&
          e->block() == previous->block() &&
          e->func() == previous->func()) {
         // Merge!
         previous->size_ += e->size_;
         // update relocToOrig_
         delete e;
         return;
      }
   }
   
   trackers_.push_back(e);
}

void CodeTracker::createIndices() {
  // Take each thing in trackers_ and add it to 
  // the origToReloc_ and relocToOrig_ mapping trees

   // We may have multiple Trackers for the same original address.
   // If they have different types that's fine; if they have the same type
   // we accumulate instead of adding one of them.

   for (TrackerList::iterator iter = trackers_.begin();
        iter != trackers_.end(); ++iter) {
      TrackerElement *e = *iter;

      relocToOrig_.insert(e->reloc(), e->reloc() + e->size(), e);
      
      if (e->type() == TrackerElement::instrumentation) {
         InstTracker *inst = static_cast<InstTracker *>(e);
         
         origToReloc_[e->block()->start()]
            [e->func() ? e->func()->addr() : 0]
            [e->orig()].instrumentation[inst->baseT()->point()] = e->reloc();
      }
      else if (e->type() == TrackerElement::padding) {
         origToReloc_[e->block()->start()][e->func() ? e->func()->addr() : 0][e->orig()].pad = e->reloc();
      }
      else {
         origToReloc_[e->block()->start()][e->func() ? e->func()->addr() : 0][e->orig()].instruction = e->reloc();
      }
   }

}

void CodeTracker::debug() {
  cerr << "************ FORWARD MAPPING ****************" << endl;

  for (ForwardMap::const_iterator iter = origToReloc_.begin();
       iter != origToReloc_.end(); ++iter) {
     for (FwdMapMiddle::const_iterator iter2 = iter->second.begin();
          iter2 != iter->second.end(); ++iter2) {
        for (FwdMapInner::const_iterator iter3 = iter2->second.begin();
             iter3 != iter2->second.end(); ++iter3) {
           cerr << "\t\t" << hex \
                << iter3->first << " "
                << iter3->second.instruction << "(insn)"
                << iter3->second.instrumentation.size() << " (bts)"
                << ", block @" << iter->first
                << ", func @" << iter2->first << dec << endl;
        }
     }
  }
     
  cerr << "************ REVERSE MAPPING ****************" << endl;

  std::vector<ReverseMap::Entry> reverseEntries;
  relocToOrig_.elements(reverseEntries);
  
  for (unsigned i = 0; i < reverseEntries.size(); ++i) {
     cerr << "\t" << hex << reverseEntries[i].first.first << "-" 
          << reverseEntries[i].first.second << ": " 
          << *(reverseEntries[i].second) << dec << endl;
  }

  cerr << endl;
}

namespace Dyninst
{
    namespace Relocation
    {
        std::ostream &operator<<(std::ostream &os, const Dyninst::Relocation::TrackerElement &e) {
            os << "Tracker(" << hex
               << e.orig() << "," << e.reloc()
               << "," << dec << e.size();
            switch(e.type()) {
                case TrackerElement::original:
                    os << ",o";
                    break;
                case TrackerElement::emulated:
                    os << ",e";
                    break;
                case TrackerElement::instrumentation:
                    os << ",i";
                    break;
                case TrackerElement::padding:
                    os << ",p";
                    break;
                default:
                    os << ",?";
                    break;
            }
            os << "," << e.block()->start() << "," << (e.func() ? e.func()->name() : "<NOFUNC>");
            os << ")" << dec;
            return os;
        }

    }
}
