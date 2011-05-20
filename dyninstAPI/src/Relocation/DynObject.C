/* Plugin Implementation */

#include "DynObject.h"
#include "PatchMgr.h"


using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::PatchAPI::DynObject;
using Dyninst::PatchAPI::DynObjectPtr;
using Dyninst::PatchAPI::InstanceSet;
using Dyninst::PatchAPI::InstancePtr;

DynObject::DynObject(ParseAPI::CodeObject* co, Address base)
  : PatchObject(co, base) {
}

DynObject::~DynObject() {
}

bool DynObject::instrument(InstanceSet* insertion_set,
                        InstanceSet* deletion_set,
                        FuncRepMap*  func_rep,
                        CallRepMap*  call_rep,
                        CallRemoval* call_removal) {
  patch_cerr << ws8 << "- A object: " << insertion_set->size() <<
    " ins, " << deletion_set->size() <<
    " del, " << func_rep->size() <<
    " func rep, " << call_rep->size() <<
    " call rep, " << call_removal->size() <<
    " call rm\n";

  if (insertion_set->size() == 0 &&
      deletion_set->size() == 0 &&
      func_rep->size() == 0 &&
      call_rep->size() == 0 &&
      call_removal->size() == 0)
    return true;
  return true;
}
