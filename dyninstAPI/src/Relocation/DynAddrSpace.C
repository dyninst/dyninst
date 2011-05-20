/* Plugin Implementation */

#include "DynAddrSpace.h"
#include "DynObject.h"

using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::DynAddrSpacePtr;
using Dyninst::PatchAPI::DynObject;

bool DynAddrSpace::loadLibrary(DynObject* obj) {
  loadObject(obj);
  if (obj) {
    coas_map_[obj->co()] = obj->as();
    return false;
  }
  return true;
}

bool DynAddrSpace::initAs(DynObject* obj) {
  init(obj);
  first_as_ = obj->as();
  coas_map_[obj->co()] = obj->as();
  return true;
}

DynAddrSpace::DynAddrSpace()
  : AddrSpace(), recursive_(false) {
}

DynAddrSpacePtr DynAddrSpace::create(DynObject* obj) {
  DynAddrSpacePtr ret = DynAddrSpacePtr(new DynAddrSpace());
  if (!ret) return DynAddrSpacePtr();
  ret->initAs(obj);
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
