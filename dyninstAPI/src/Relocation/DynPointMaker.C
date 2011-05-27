#include "DynPointMaker.h"
#include "function.h"
#include "instPoint.h"


Point* DynPointMaker::createPoint(Address addr, Point::Type type,
                                  Address* scope) {
  // instPoint* ret = new instPoint(addr, type, mgr_, SCAST_FI(scope));
  return NULL;
}

Point* DynPointMaker::createPoint(Address addr, Point::Type type,
                                  PatchBlock* scope) {
  instPoint* ret = new instPoint(addr, type, mgr_, SCAST_BI(scope));
  return ret;
}

Point* DynPointMaker::createPoint(Address addr, Point::Type type,
                                  PatchEdge* scope) {
  return NULL;
}

Point* DynPointMaker::createPoint(Address addr, Point::Type type,
                                  PatchFunction* scope) {
  instPoint* ret = new instPoint(addr, type, mgr_, SCAST_FI(scope));
  return ret;
}
