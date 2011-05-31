#include "DynInstrumenter.h"

using Dyninst::PatchAPI::DynInstrumenter;

bool DynInstrumenter::process() {
  DynAddrSpacePtr das = DYN_CAST(DynAddrSpace, as_);
  std::set<AddressSpace*> seen;
  bool ret = true;
  for (DynAddrSpace::AsSet::iterator i = das->asSet().begin();
       i != das->asSet().end(); i++) {
    AddressSpace* as = *i;
    if (std::find(seen.begin(), seen.end(), as) == seen.end()) {
      seen.insert(as);
      if (!as->relocate()) { ret = false; }
    }
  }
  return ret;
}

