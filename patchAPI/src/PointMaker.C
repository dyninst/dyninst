#include "Point.h"
#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               PatchMgrPtr mgr,
                               Address* scope) {
  Point* ret = new Point(addr, type, mgr, scope);
  return ret;
}

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               PatchMgrPtr mgr,
                               PatchBlock* scope) {
  Point* ret = new Point(addr, type, mgr, scope);
  return ret;
}

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               PatchMgrPtr mgr,
                               PatchEdge* scope) {
  Point* ret = new Point(addr, type, mgr, scope);
  return ret;
}

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               PatchMgrPtr mgr,
                               PatchFunction* scope) {
  Point* ret = new Point(addr, type, mgr, scope);
  return ret;
}
