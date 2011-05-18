/* Plugin Implementation */

#include "DynAddrSpace.h"
#include "DynObject.h"

using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::DynAddrSpacePtr;

bool DynAddrSpace::loadLibrary(PatchObjectPtr obj, AddressSpace* as) {
  DynObjectPtr dobj = DYN_CAST(DynObject, obj);
  loadObject(dobj);
  if (dobj) {
    coas_map_[dobj->co()] = as;
    dobj->setAs(as);
    return obj;
  }
  return true;
}

bool DynAddrSpace::initAs(PatchObjectPtr obj, AddressSpace* as) {
  DynObjectPtr dobj = DYN_CAST(DynObject, obj);
  init(dobj);
  first_as_ = as;
  coas_map_[dobj->co()] = as;
  dobj->setAs(as);
  return true;
}

DynAddrSpace::DynAddrSpace()
  : AddrSpace(), recursive_(false) {
}

DynAddrSpacePtr DynAddrSpace::create(PatchObjectPtr obj, AddressSpace* as) {
  DynAddrSpacePtr ret = DynAddrSpacePtr(new DynAddrSpace());
  if (!ret) return DynAddrSpacePtr();
  ret->initAs(obj, as);
  return ret;
}

bool DynAddrSpace::write(PatchObjectPtr obj, Address to, Address from, size_t size) {
  DynObjectPtr dobj = dyn_detail::boost::dynamic_pointer_cast<DynObject>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  return dobj->as()->writeDataSpace(reinterpret_cast<void*>(to), size,
                                     reinterpret_cast<void*>(from));
}

Address DynAddrSpace::malloc(PatchObjectPtr obj, size_t size, Address near) {
  DynObjectPtr dobj = dyn_detail::boost::dynamic_pointer_cast<DynObject>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  return dobj->as()->inferiorMalloc(size, anyHeap, near, NULL);
}

bool DynAddrSpace::realloc(PatchObjectPtr obj, Address orig, size_t size) {
  DynObjectPtr dobj = dyn_detail::boost::dynamic_pointer_cast<DynObject>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  return dobj->as()->inferiorRealloc(orig, size);
}

bool DynAddrSpace::free(PatchObjectPtr obj, Address orig) {
  DynObjectPtr dobj = dyn_detail::boost::dynamic_pointer_cast<DynObject>(obj);
  if (coobj_map_[dobj->co()] != dobj) return false;
  dobj->as()->inferiorFree(orig);
  return true;
}
