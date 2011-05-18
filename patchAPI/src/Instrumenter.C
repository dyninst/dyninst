/* Plugin */

#include "Instrumenter.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::Instrumenter;
using Dyninst::PatchAPI::InstanceSet;

InstrumenterPtr Instrumenter::create(AddrSpacePtr as) {
  InstrumenterPtr ret = InstrumenterPtr(new Instrumenter(as));
  if (!ret) return InstrumenterPtr();
  return ret;
}

bool Instrumenter::process(InstanceSet* insertion_set,
                           InstanceSet* deletion_set,
                           FuncRepMap*  func_rep,
                           CallRepMap*  call_rep,
                           CallRemoval* call_removal) {
  // In each iteration, we only instrument a particular object
  for (AddrSpace::CoObjMap::iterator ci = as_->getCoobjMap().begin();
       ci != as_->getCoobjMap().end(); ci++) {
    PatchObjectPtr obj = (*ci).second;
    InstanceSet i_set;
    InstanceSet d_set;
    FuncRepMap f_rep;
    CallRepMap c_rep;
    CallRemoval c_removal;

    for (InstanceSet::iterator ii = insertion_set->begin();
         ii != insertion_set->end(); ii++) {
      if ((*ii)->point()->obj() == obj) i_set.insert(*ii);
    }

    for (InstanceSet::iterator di = deletion_set->begin();
         di != deletion_set->end(); di++) {
      if ((*di)->point()->obj() == obj) d_set.insert(*di);
    }

    for (FuncRepMap::iterator fi = func_rep->begin();
         fi != func_rep->end(); fi++) {
      if ((*fi).first->object() == obj) f_rep[(*fi).first] = (*fi).second;
    }

    for (CallRepMap::iterator ci = call_rep->begin();
         ci != call_rep->end(); ci++) {
      if ((*ci).first->obj() == obj) c_rep[(*ci).first] = (*ci).second;
    }

    for (CallRemoval::iterator cri = call_removal->begin();
         cri != call_removal->end(); cri++) {
      if ((*cri)->obj() == obj) c_removal.insert(*cri);
    }

    // Here we go!
    obj->process(&i_set, &d_set, &f_rep, &c_rep, &c_removal);
  }
  return true;
}
