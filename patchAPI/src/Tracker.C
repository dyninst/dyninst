/* Utility */

#include "Tracker.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::Tracker;
using Dyninst::PatchAPI::Element;

bool Tracker::origToReloc(Address origAddr,
                          PatchFunction *func,
                          RelocatedElements &reloc) const {
  BFM_citer iter = origToReloc_.find(func->addr());
  if (iter == origToReloc_.end()) return false;
  const ForwardsMap &fm = iter->second;
  FM_citer iter2 = fm.find(origAddr);
  if (iter2 == fm.end()) return false;
  reloc = iter2->second;
  return true;
}

Element *Tracker::findByReloc(Address addr) const {
  Element *e = NULL;
  if (!relocToOrig_.find(addr, e))
    return NULL;
  return e;
}

void Tracker::addTracker(Element *e) {
  // We should look into being more efficient by collapsing ranges
  // of relocated code into a single OriginalTracker

  // If that happens, the assumption origToReloc makes that we can
  // get away without an IntervalTree will be violated and a lot
  // of code will need to be rewritten.
   if (!trackers_.empty()) {
      Element *last = trackers_.back();
      if (e->orig() == last->orig() &&
          e->type() == last->type()) {
         if (false) patch_cerr << "OVERLAPPING TRACKERS, combining...." << endl;
                 if (false) patch_cerr << "\t Current: " << *last << endl;
                 if (false) patch_cerr << "\t New: " << *e << endl;
         assert(e->reloc() == (last->reloc() + last->size()));
         last->setSize(last->size() + e->size());
         return;
      }
   }
   if (false) patch_cerr << "Adding tracker: " << *e << endl;

   trackers_.push_back(e);
}

void Tracker::createIndices() {
  // Take each thing in trackers_ and add it to
  // the origToReloc_ and relocToOrig_ mapping trees
  for (TrackerList::iterator iter = trackers_.begin();
       iter != trackers_.end(); ++iter) {
    Element *e = *iter;

    relocToOrig_.insert(e->reloc(), e->reloc() + e->size(), e);

   if (e->type() == Element::instrumentation) {
     origToReloc_[e->block()->function()->addr()][e->orig()].instrumentation = e->reloc();
   }
   else {
     origToReloc_[e->block()->function()->addr()][e->orig()].instruction = e->reloc();
   }
  }
}

void Tracker::debug() {
  cerr << "************ FORWARD MAPPING ****************" << endl;
  for (BlockForwardsMap::iterator bfm_iter = origToReloc_.begin();
       bfm_iter != origToReloc_.end(); ++bfm_iter) {
     cerr << "\t" << bfm_iter->first << endl;
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

std::ostream &operator<<(std::ostream &os, const Dyninst::PatchAPI::Element &e) {
  os << "Tracker(" << hex
     << e.orig() << "," << e.reloc()
     << "," << dec << e.size();
  switch(e.type()) {
  case Element::original:
    os << ",o";
    break;
  case Element::emulated:
    os << ",e";
    break;
  case Element::instrumentation:
    os << ",i";
    break;
  default:
    os << ",?";
    break;
  }
  os << "," << e.block()->start() << "," << e.block()->function()->name();
  os << ")" << dec;
  return os;
}
