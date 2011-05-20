/* Public Interface */

#include "AddrSpace.h"
#include "PatchObject.h"

using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::AddrSpacePtr;

/* Use an PatchObject to initialize the AddrSpace */
bool AddrSpace::init(PatchObject* obj) {
  first_object_ = obj;
  loadObject(obj);
  return true;
}

AddrSpacePtr AddrSpace::create(PatchObject* obj) {
  AddrSpacePtr ret = AddrSpacePtr(new AddrSpace());
  if (!ret) return AddrSpacePtr();
  ret->init(obj);
  return ret;
}

bool AddrSpace::loadObject(PatchObject* obj) {
  coobj_map_[obj->co()] = obj;
  obj->setAddrSpace(shared_from_this());
  return true;
}

AddrSpace::~AddrSpace() {
  for (CoObjMap::iterator i = coobj_map_.begin(); i != coobj_map_.end(); i++) {
    PatchObject* obj = (*i).second;
    PatchObject::destroy(obj);
  }
}
