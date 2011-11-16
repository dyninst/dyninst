#include "common/h/Types.h"
#include "Point.h"
#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst::PatchAPI;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchLocation;
using Dyninst::PatchAPI::PatchMgrPtr;

Point *
PointMaker::createPoint(PatchLocation loc, Point::Type t) {
   switch(t) {
      case Point::PreInsn:
      case Point::PostInsn:
         return mkInsnPoint(t, mgr_, loc.block, loc.addr, loc.insn, loc.func);
         break;
      case Point::BlockEntry:
      case Point::BlockExit:
      case Point::BlockDuring:
         return mkBlockPoint(t, mgr_, loc.block, loc.func);
         break;
      case Point::FuncEntry:
      case Point::FuncDuring:
         return mkFuncPoint(t, mgr_, loc.func);
         break;
      case Point::FuncExit:
      case Point::PreCall:
      case Point::PostCall:
         return mkFuncSitePoint(t, mgr_, loc.func, loc.block);
         break;
      case Point::EdgeDuring:
         return mkEdgePoint(t, mgr_, loc.edge, loc.func);
         break;
      default:
         assert(0 && "Unimplemented!");
         return NULL;
   }
}

Point *PointMaker::mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f) {
   return new Point(t, m, f);
}
Point *PointMaker::mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *f, PatchBlock *b) {
   return new Point(t, m, f, b);
}
Point *PointMaker::mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b, PatchFunction *f) {
   return new Point(t, m, b, f);
}
Point *PointMaker::mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b, 
                               Address a, InstructionAPI::Instruction::Ptr i, PatchFunction *f) {
   return new Point(t, m, b, a, i, f);
}
Point *PointMaker::mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *e, PatchFunction *f) {
   return new Point(t, m, e, f);
}
