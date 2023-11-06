/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "Point.h"
#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst::PatchAPI;
using Dyninst::PatchAPI::Location;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::Location;
using Dyninst::PatchAPI::PatchMgrPtr;

Point *
PointMaker::createPoint(Location loc, Point::Type t) {
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
                               Address a, InstructionAPI::Instruction i, PatchFunction *f) {
   return new Point(t, m, b, a, i, f);
}
Point *PointMaker::mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *e, PatchFunction *f) {
   return new Point(t, m, e, f);
}
