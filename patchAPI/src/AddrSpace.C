/* Public Interface */

#include "AddrSpace.h"
#include "PatchObject.h"

using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::AddrSpacePtr;

/* Use an PatchObject (a.out) to initialize the AddrSpace */

bool
AddrSpace::init(PatchObject* obj) {
  first_object_ = obj;
  loadObject(obj);
  return true;
}

AddrSpacePtr
AddrSpace::create(PatchObject* obj) {
  AddrSpacePtr ret = AddrSpacePtr(new AddrSpace());
  if (!ret) return AddrSpacePtr();
  ret->init(obj);
  return ret;
}

bool
AddrSpace::loadObject(PatchObject* obj) {
  obj_set_.insert(obj);
  obj->setAddrSpace(shared_from_this());
  return true;
}

AddrSpace::~AddrSpace() {
  for (ObjSet::iterator i = obj_set_.begin(); i != obj_set_.end(); i++) {
    PatchObject* obj = *i;
    delete obj;
  }
}
