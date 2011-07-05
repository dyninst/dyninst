/* Public Interface */

#include "AddrSpace.h"
#include "PatchObject.h"

using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::PatchObject;

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
  obj_map_[obj->co()] = obj;
  obj->setAddrSpace(shared_from_this());
  return true;
}

AddrSpace::~AddrSpace() {
  std::cerr << obj_map_.size() << " objects\n";
  for (ObjMap::iterator i = obj_map_.begin(); i != obj_map_.end(); i++) {
    PatchObject* obj = i->second;
    delete obj;
  }
}

bool
AddrSpace::write(PatchObject* /*obj*/, Address /*to*/,
                 Address /*from*/, size_t /*size*/) {
  return false;
}

Address
AddrSpace::malloc(PatchObject* /*obj*/, size_t /*size*/,
                  Address /*near*/) {
  return false;
}

bool
AddrSpace::realloc(PatchObject* /*obj*/, Address /*orig*/,
                   size_t /*size*/) {
  return false;
}

bool
AddrSpace::free(PatchObject* /*obj*/, Address /*orig*/) {
  return false;
}

std::string AddrSpace::format() const { 
   stringstream ret;
   ret << hex << this << dec << endl;
   return ret.str();
}

PatchObject *AddrSpace::findObject(const ParseAPI::CodeObject *co) const
{
    ObjMap::const_iterator oit = obj_map_.find(co);
    if (oit != obj_map_.end()) {
        return oit->second;
    }
    return NULL;
}