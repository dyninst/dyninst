#include "Point.h"
#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               Address* scope) {
  Point* ret = new Point(addr, type, mgr_, scope);
  return ret;
}

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               PatchBlock* scope) {
  Point* ret = new Point(addr, type, mgr_, scope);
  return ret;
}

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               PatchEdge* scope) {
  Point* ret = new Point(addr, type, mgr_, scope);
  return ret;
}

Point* PointMaker::createPoint(Address addr,
                               Point::Type type,
                               PatchFunction* scope) {
  Point* ret = new Point(addr, type, mgr_, scope);
  return ret;
}
