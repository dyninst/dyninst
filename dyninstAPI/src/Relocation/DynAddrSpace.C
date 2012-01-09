/* Plugin Implementation */

#include "DynAddrSpace.h"
#include "DynObject.h"

using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::DynObject;
using Dyninst::PatchAPI::PatchObject;

bool DynAddrSpace::loadLibrary(DynObject* obj) {
  if (obj) {
    loadObject(obj);
    as_set_.insert(obj->as());
    return false;
  }
  return true;
}

bool DynAddrSpace::initAs(DynObject* obj) {
  init(obj);
  first_as_ = obj->as();
  as_set_.insert(obj->as());
  return true;
}

DynAddrSpace::DynAddrSpace()
  : AddrSpace(), recursive_(false) {
}

DynAddrSpace* DynAddrSpace::create(DynObject* obj) {
  DynAddrSpace* ret = new DynAddrSpace();
  if (!ret) return NULL;
  ret->initAs(obj);
  return ret;
}

bool DynAddrSpace::write(PatchObject* obj, Address to, Address from, size_t size) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  return dobj->as()->writeDataSpace(reinterpret_cast<void*>(to), size,
                                     reinterpret_cast<void*>(from));
}

Address DynAddrSpace::malloc(PatchObject* obj, size_t size, Address /*near*/) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  return dobj->as()->inferiorMalloc(size);
}

bool DynAddrSpace::realloc(PatchObject* obj, Address orig, size_t size) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  return dobj->as()->inferiorRealloc(orig, size);
}

bool DynAddrSpace::free(PatchObject* obj, Address orig) {
  DynObject* dobj = dynamic_cast<DynObject*>(obj);
  dobj->as()->inferiorFree(orig);
  return true;
}
