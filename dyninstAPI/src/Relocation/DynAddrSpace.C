/* Plugin Implementation */

#include "DynAddrSpace.h"
#include "DynObject.h"

using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::DynAddrSpacePtr;
using Dyninst::PatchAPI::DynObject;

bool DynAddrSpace::loadLibrary(PatchObject* obj, AddressSpace* as) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  loadObject(dobj);
  if (dobj) {
    coas_map_[dobj->co()] = as;
    dobj->setAs(as);
    return obj;
  }
  return true;
}

bool DynAddrSpace::initAs(PatchObject* obj, AddressSpace* as) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  init(dobj);
  first_as_ = as;
  coas_map_[dobj->co()] = as;
  dobj->setAs(as);
  return true;
}

DynAddrSpace::DynAddrSpace()
  : AddrSpace(), recursive_(false) {
}

DynAddrSpacePtr DynAddrSpace::create(PatchObject* obj, AddressSpace* as) {
  DynAddrSpacePtr ret = DynAddrSpacePtr(new DynAddrSpace());
  if (!ret) return DynAddrSpacePtr();
  ret->initAs(obj, as);
  return ret;
}

bool DynAddrSpace::write(PatchObject* obj, Address to, Address from, size_t size) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  return dobj->as()->writeDataSpace(reinterpret_cast<void*>(to), size,
                                     reinterpret_cast<void*>(from));
}

Address DynAddrSpace::malloc(PatchObject* obj, size_t size, Address near) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  return dobj->as()->inferiorMalloc(size, anyHeap, near, NULL);
}

bool DynAddrSpace::realloc(PatchObject* obj, Address orig, size_t size) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  return dobj->as()->inferiorRealloc(orig, size);
}

bool DynAddrSpace::free(PatchObject* obj, Address orig) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  dobj->as()->inferiorFree(orig);
  return true;
}
