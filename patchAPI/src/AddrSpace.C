/* Public Interface */

#include "AddrSpace.h"
#include "Object.h"

using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::AddrSpacePtr;

/* Use an Object to initialize the AddrSpace */
bool AddrSpace::init(ObjectPtr obj) {
  first_object_ = obj;
  loadObject(obj);
  return true;
}

AddrSpacePtr AddrSpace::create(ObjectPtr obj) {
  AddrSpacePtr ret = AddrSpacePtr(new AddrSpace());
  if (!ret) return AddrSpacePtr();
  ret->init(obj);
  return ret;
}

bool AddrSpace::loadObject(ObjectPtr obj) {
  coobj_map_[obj->co()] = obj;
  obj->setAs(shared_from_this());
  return true;
}

AddrSpace::~AddrSpace() {
  for (CoObjMap::iterator i = coobj_map_.begin(); i != coobj_map_.end(); i++) {
    ObjectPtr obj = (*i).second;
    Object::destroy(obj);
  }
}
