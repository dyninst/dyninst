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

#include "CodeTracker.h"
#include "dyninstAPI/src/function.h" // for debug purposes
#include "dyninstAPI/src/debug.h"

#include <iostream>

using namespace Dyninst;
using namespace Relocation;
using namespace std;

bool CodeTracker::origToReloc(Address origAddr,
			      int_function *func,
			      RelocatedElements &reloc) const {
  BFM_citer iter = origToReloc_.find(func);
  if (iter == origToReloc_.end()) return false;

  const ForwardsMap &fm = iter->second;
  FM_citer iter2 = fm.find(origAddr);
  if (iter2 == fm.end()) return false;

  reloc = iter2->second;

  return true;
}

bool CodeTracker::relocToOrig(Address relocAddr, 
			      Address &orig, 
			      int_function *&func,
                              baseTrampInstance *&bti) const {
  TrackerElement *e = NULL;
  if (!relocToOrig_.find(relocAddr, e))
    return false;
  orig = e->relocToOrig(relocAddr);
  func = e->func();
  if (e->type() == TrackerElement::instrumentation) {
     InstTracker *i = static_cast<InstTracker *>(e);
     bti = i->baseT();
  }

  return true;
}

void CodeTracker::addTracker(TrackerElement *e) {
  // We should look into being more efficient by collapsing ranges
  // of relocated code into a single OriginalTracker

  // If that happens, the assumption origToReloc makes that we can
  // get away without an IntervalTree will be violated and a lot
  // of code will need to be rewritten.
   assert(e->func());
  trackers_.push_back(e);
}

void CodeTracker::createIndices() {
  // Take each thing in trackers_ and add it to 
  // the origToReloc_ and relocToOrig_ mapping trees
  for (TrackerList::iterator iter = trackers_.begin();
       iter != trackers_.end(); ++iter) {
    TrackerElement *e = *iter;

    relocToOrig_.insert(e->reloc(), e->reloc() + e->size(), e);

    if (e->func()) {
       if (e->type() == TrackerElement::instrumentation) {
          origToReloc_[e->func()][e->orig()].instrumentation = e->reloc();
       }
       else {
          origToReloc_[e->func()][e->orig()].instruction = e->reloc();
       }
    }
    else {
       cerr << "WARNING: ignoring tracker entry " << (*e) << endl;
    }
  }

  if (dyn_debug_reloc) debug();
}

void CodeTracker::debug() {
  cerr << "************ FORWARD MAPPING ****************" << endl;

  for (BlockForwardsMap::iterator bfm_iter = origToReloc_.begin(); 
       bfm_iter != origToReloc_.end(); ++bfm_iter) {
     cerr << "\t" << bfm_iter->first->symTabName() << endl;
     for (ForwardsMap::iterator fm_iter = bfm_iter->second.begin();
          fm_iter != bfm_iter->second.end(); ++fm_iter) {
        cerr << "\t\t" << hex << fm_iter->first << " -> "
             << fm_iter->second.instrumentation << "(instrumentation), " 
             << fm_iter->second.instruction << "(instruction)" << dec << endl;
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
  default:
    os << ",?";
    break;
  }
  os << ")" << dec;
  return os;
}
