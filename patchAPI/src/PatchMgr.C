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
/* Public Interface */

#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"
#include "Point.h"
#include "PatchCallback.h"
#include "compiler_annotations.h"

#include "dyninstversion.h"

using namespace Dyninst;
using namespace PatchAPI;

using Dyninst::ParseAPI::CodeSource;
using Dyninst::InstructionAPI::InstructionDecoder;
using Dyninst::InstructionAPI::Instruction;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PointSet;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchFunction;

static const int PatchAPI_major_version = DYNINST_MAJOR_VERSION;
static const int PatchAPI_minor_version = DYNINST_MINOR_VERSION;
static const int PatchAPI_maintenance_version = DYNINST_PATCH_VERSION;

void PatchMgr::version(int& major, int& minor, int& maintenance)
{
    major = PatchAPI_major_version;
    minor = PatchAPI_minor_version;
    maintenance = PatchAPI_maintenance_version;
}

PatchMgr::PatchMgr(AddrSpace* as, Instrumenter* inst, PointMaker* pt)
  : as_(as) {
  if (!pt) {
    patchapi_debug("Use default PointMaker");
    point_maker_ = new PointMaker;
  } else {
    patchapi_debug("Use plugin PointMaker");
    point_maker_ = pt;
  }
  if (!inst) {
    patchapi_debug("Use default Instrumenter");
    instor_ = Instrumenter::create(as);
  } else {
    patchapi_debug("Use plugin Instrumenter");
    inst->setAs(as);
    instor_ = inst;
  }
}

PatchMgrPtr
PatchMgr::create(AddrSpace* as, Instrumenter* inst, PointMaker* pf) {
  patchapi_debug("Create PatchMgr");
  PatchMgrPtr ret = PatchMgrPtr(new PatchMgr(as, inst, pf));
  if (!ret) return PatchMgrPtr();
  ret->as_->mgr_ = ret;
  ret->pointMaker()->setMgr(ret);
  return ret;
}

/* Return false if no point is found */
bool
PatchMgr::removeSnippet(InstancePtr instance) {
  if (instance == InstancePtr()) return false;
  return instance->destroy();
}

Point *PatchMgr::findPoint(Location loc,
                           Point::Type type,
                           bool create) {
   // Verify an untrusted Location
   if (!loc.trusted) {
      if (!verify(loc)) return NULL;
   }

   // Not sure if it's better to go by type
   // or location first, so we're running
   // with type...
   switch (type) {
      case Point::PreInsn:
      case Point::PostInsn:
      case Point::BlockEntry:
      case Point::BlockExit:
      case Point::BlockDuring:
         if (loc.func) {
            return loc.func->findPoint(loc, type, create);
         }
         else {
            if (!loc.block) return NULL;
            return loc.block->findPoint(loc, type, create);
         }
         break;
      case Point::PreCall:
      case Point::PostCall:
      case Point::FuncExit:
         if (!loc.func || !loc.block) return NULL;
         return loc.func->findPoint(loc, type, create);
         break;
      case Point::FuncEntry:
      case Point::FuncDuring:
         if (!loc.func) {
            return NULL;
         }
         return loc.func->findPoint(loc, type, create);
         break;
      case Point::EdgeDuring:
         if (loc.func) {
            return loc.func->findPoint(loc, type, create);
         }
         else {
            // Not doing generic edges yet...
            return NULL;
         }
         break;
         // These are unimplemented
      case Point::LoopStart:
      case Point::LoopEnd:
      case Point::LoopIterStart:
      case Point::LoopIterEnd:
      default:
         return NULL;
   }
}

PatchMgr::~PatchMgr() {
  patchapi_debug("Destroy PatchMgr");
  delete as_;
  delete point_maker_;
  delete instor_;
  
}

bool PatchMgr::getCandidates(Scope &scope,
                             Point::Type types,
                             Candidates &ret) {
   // We hand in pre-generated lists to reduce the requirement for iteration
   if (wantFuncs(scope, types)) getFuncCandidates(scope, types, ret);
   if (wantCallSites(scope, types)) getCallSiteCandidates(scope, types, ret);
   if (wantExitSites(scope, types)) getExitSiteCandidates(scope, types, ret);

   if (wantBlocks(scope, types)) getBlockCandidates(scope, types, ret);
   if (wantEdges(scope, types)) getEdgeCandidates(scope, types, ret);
   if (wantInsns(scope, types)) getInsnCandidates(scope, types, ret);

   if (wantBlockInstances(scope, types)) getBlockInstanceCandidates(scope, types, ret);
   //if (wantEdgeInstances(scope, types)) getEdgeInstanceCandidates(scope, types, ret);
   if (wantInsnInstances(scope, types)) getInsnInstanceCandidates(scope, types, ret);

   return true;
}

bool PatchMgr::wantFuncs(Scope &scope, Point::Type types) {
  return (Point::TestType(types, Point::FuncDuring) ||
          (Point::TestType(types, Point::FuncEntry) &&
           (scope.func || scope.obj || scope.wholeProgram)));
}

bool PatchMgr::wantCallSites(Scope &scope, Point::Type types) {
  return (Point::TestType(types, Point::CallTypes) &&
         (scope.func || scope.obj || scope.wholeProgram));
}

bool PatchMgr::wantExitSites(Scope &scope, Point::Type types) {
  return (Point::TestType(types, Point::FuncExit) &&
         (scope.func || scope.obj || scope.wholeProgram));
}

bool PatchMgr::wantBlocks(Scope &scope, Point::Type types) {
  return (scope.func == NULL && Point::TestType(types, Point::BlockTypes));
}

bool PatchMgr::wantBlockInstances(Scope &scope, Point::Type types) {
  return (scope.func != NULL && Point::TestType(types, Point::BlockTypes));
}

bool PatchMgr::wantEdges(Scope &scope, Point::Type types) {
  return (scope.func == NULL && Point::TestType(types, Point::EdgeTypes));
}

bool PatchMgr::wantInsns(Scope &scope, Point::Type types) {
  return (scope.func == NULL && Point::TestType(types, Point::InsnTypes));
}

bool PatchMgr::wantInsnInstances(Scope &scope, Point::Type types) {
  return (scope.func != NULL && Point::TestType(types, Point::InsnTypes));
}

void PatchMgr::getFuncCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   // We can either have a scope of PatchObject and be looking for all
   // the functions it contains, a scope of a Function, or be looking
   // for every single function we know about.
   Functions funcs;
   getFuncs(scope, funcs);

   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      if (types & Point::FuncDuring) ret.push_back(Candidate(Location::Function(*iter), Point::FuncDuring));
      if (types & Point::FuncEntry) ret.push_back(Candidate(Location::EntrySite(*iter, (*iter)->entry(), true), Point::FuncEntry));
   }
}


void PatchMgr::getCallSiteCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   CallSites sites;
   getCallSites(scope, sites);
   for (CallSites::iterator iter = sites.begin(); iter != sites.end(); ++iter) {
      if (types & Point::PreCall) ret.push_back(Candidate(Location::CallSite(*iter), Point::PreCall));
      if (types & Point::PostCall) ret.push_back(Candidate(Location::CallSite(*iter), Point::PostCall));
   }
}

void PatchMgr::getExitSiteCandidates(Scope &scope, Point::Type, Candidates &ret) {
   ExitSites sites;
   getExitSites(scope, sites);
   for (ExitSites::iterator iter = sites.begin(); iter != sites.end(); ++iter) {
      ret.push_back(Candidate(Location::ExitSite(*iter), Point::FuncExit));
   }
}

void PatchMgr::getBlockCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   Blocks blocks;
   getBlocks(scope, blocks);
   for (Blocks::iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      if (types & Point::BlockEntry) ret.push_back(Candidate(Location::Block(*iter), Point::BlockEntry));
      if (types & Point::BlockDuring) ret.push_back(Candidate(Location::Block(*iter), Point::BlockDuring));
      if (types & Point::BlockExit) ret.push_back(Candidate(Location::Block(*iter), Point::BlockExit));
   }
}

void PatchMgr::getEdgeCandidates(Scope &scope, Point::Type, Candidates &ret) {
   Edges edges;
   getEdges(scope, edges);
   for (Edges::iterator iter = edges.begin(); iter != edges.end(); ++iter) {
      ret.push_back(Candidate(Location::Edge(*iter), Point::EdgeDuring));
   }
}

void PatchMgr::getInsnCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   Insns insns;
   getInsns(scope, insns);
   for (Insns::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      if (types & Point::PreInsn) ret.push_back(Candidate(Location::Instruction(*iter), Point::PreInsn));
      if (types & Point::PostInsn) ret.push_back(Candidate(Location::Instruction(*iter), Point::PostInsn));
   }
}

void PatchMgr::getBlockInstanceCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   BlockInstances blocks;
   getBlockInstances(scope, blocks);
   for (BlockInstances::iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      if (types & Point::BlockEntry) ret.push_back(Candidate(Location::BlockInstance(iter->first, iter->second, true), Point::BlockEntry));
      if (types & Point::BlockDuring) ret.push_back(Candidate(Location::BlockInstance(iter->first, iter->second, true), Point::BlockDuring));
      if (types & Point::BlockExit) ret.push_back(Candidate(Location::BlockInstance(iter->first, iter->second, true), Point::BlockExit));
   }
}

void PatchMgr::getInsnInstanceCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   InsnInstances insns;
   getInsnInstances(scope, insns);
   for (InsnInstances::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      if (types & Point::PreInsn) ret.push_back(Candidate(Location::InstructionInstance(iter->first, iter->second, true), Point::PreInsn));
      if (types & Point::PostInsn) ret.push_back(Candidate(Location::InstructionInstance(iter->first, iter->second, true), Point::PostInsn));
   }
}

void PatchMgr::getFuncs(Scope &scope, Functions &funcs) {
   if (scope.wholeProgram) {
      AddrSpace::ObjMap &objs = as()->objMap();
      for (AddrSpace::ObjMap::iterator iter = objs.begin(); iter != objs.end(); ++iter) {
         iter->second->funcs(std::back_inserter(funcs));
      }
   }
   else if (scope.obj) {
      scope.obj->funcs(std::back_inserter(funcs));
   }
   else if (scope.func) {
      funcs.push_back(scope.func);
   }
}

void PatchMgr::getCallSites(Scope &scope, CallSites &sites) {
   // All sites in whatever functions we want
   Functions funcs;
   getFuncs(scope, funcs);
   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      const PatchFunction::Blockset &c = (*iter)->callBlocks();
      for (PatchFunction::Blockset::const_iterator iter2 = c.begin(); iter2 != c.end(); ++iter2) {
         if (!scope.block || (scope.block == *iter2))
            sites.push_back(CallSite_t(*iter, *iter2));
      }
   }
}

void PatchMgr::getExitSites(Scope &scope, ExitSites &sites) {
   // All sites in whatever functions we want
   Functions funcs;
   getFuncs(scope, funcs);
   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      const PatchFunction::Blockset &e = (*iter)->exitBlocks();
      for (PatchFunction::Blockset::const_iterator iter2 = e.begin(); iter2 != e.end(); ++iter2) {
         if (!scope.block || (scope.block == *iter2))
            sites.push_back(ExitSite_t(*iter, *iter2));
      }
   }
}

void PatchMgr::getBlocks(Scope &scope, Blocks &blocks) {
   if (scope.wholeProgram) {
      const AddrSpace::ObjMap &objs = as()->objMap();
      for (AddrSpace::ObjMap::const_iterator iter = objs.begin(); iter != objs.end(); ++iter) {
         iter->second->blocks(std::back_inserter(blocks));
      }
   }
   else if (scope.obj) {
      scope.obj->blocks(std::back_inserter(blocks));
   }
   else if (scope.block) {
      blocks.push_back(scope.block);
   }
}

void PatchMgr::getEdges(Scope &scope, Edges &edges) {
   if (scope.wholeProgram) {
      const AddrSpace::ObjMap &objs = as()->objMap();
      for (AddrSpace::ObjMap::const_iterator iter = objs.begin(); iter != objs.end(); ++iter) {
         iter->second->edges(std::back_inserter(edges));
      }
   }
   else if (scope.obj) {
      scope.obj->edges(std::back_inserter(edges));
   }
}

void PatchMgr::getInsns(Scope &scope, Insns &insns) {
   Blocks blocks;
   getBlocks(scope, blocks);

   for (Blocks::iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      PatchBlock::Insns tmp;
      (*iter)->getInsns(tmp);
      for (PatchBlock::Insns::iterator t = tmp.begin(); t != tmp.end(); ++t) {
         insns.push_back(InsnLoc_t(*iter, t->first, t->second));
      }
   }
}

void PatchMgr::getBlockInstances(Scope &scope, BlockInstances &blocks) {
   Functions funcs;
   getFuncs(scope, funcs);

   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      const PatchFunction::Blockset &b = (*iter)->blocks();
      for (PatchFunction::Blockset::const_iterator iter2 = b.begin(); iter2 != b.end(); ++iter2) {
         // TODO FIXME: make this more efficient to avoid iunnecessary iteration
         if (scope.block && scope.block != *iter2) continue;
         blocks.push_back(BlockInstance(*iter, *iter2));
      }
   }
}

void PatchMgr::getInsnInstances(Scope &scope, InsnInstances &insns) {
   Functions funcs;
   getFuncs(scope, funcs);

   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      const PatchFunction::Blockset &b = (*iter)->blocks();
      for (PatchFunction::Blockset::const_iterator iter2 = b.begin(); iter2 != b.end(); ++iter2) {
         // TODO FIXME: make this more efficient to avoid iunnecessary iteration
         if (scope.block && scope.block != *iter2) continue;
         PatchBlock::Insns i;
         (*iter2)->getInsns(i);
         for (PatchBlock::Insns::iterator iter3 = i.begin(); iter3 != i.end(); ++iter3) {
            insns.push_back(InsnInstance(*iter, InsnLoc_t(*iter2, iter3->first, iter3->second)));
         }
      }
   }
}


void PatchMgr::enumerateTypes(Point::Type types, EnumeratedTypes &out) {
   for (unsigned i = 0; i <= 31; ++i) {
      Point::Type tmp = (Point::Type) type_val(i);
      if (types & tmp) out.push_back(tmp);
   }
}


void PatchMgr::destroy(Point *p) {
   p->obj()->cb()->destroy(p);
}

bool PatchMgr::verify(Location &loc) {
   if (loc.trusted) return true;

   switch (loc.type) {
      case Location::Function_:
         break;
      case Location::Block_:
         break;
      case Location::BlockInstance_:
         if (loc.func->blocks().find(loc.block) == loc.func->blocks().end()) return false;
         break;
      case Location::InstructionInstance_:
         if (loc.func->blocks().find(loc.block) == loc.func->blocks().end()) return false;
         // Fall through to Instruction_ case for detailed checking.
         DYNINST_FALLTHROUGH;
      case Location::Instruction_:
         if (loc.addr < loc.block->start()) return false;
         if (loc.addr > loc.block->last()) return false;
         loc.insn = loc.block->getInsn(loc.addr);
         if (!loc.insn.isValid()) return false;
         break;
      case Location::Edge_:
         break;
      case Location::EdgeInstance_:
         if (loc.func->blocks().find(loc.edge->src()) == loc.func->blocks().end()) return false;
         if (loc.func->blocks().find(loc.edge->trg()) == loc.func->blocks().end()) return false;
         break;
      case Location::Entry_:
         if (loc.func->entry() != loc.block) return false;
         break;
      case Location::Call_:
         // Check to see if the block is in the call blocks
         if (loc.func->callBlocks().find(loc.block) == loc.func->callBlocks().end()) return false;
         break;
      case Location::Exit_:
         if (loc.func->exitBlocks().find(loc.block) == loc.func->exitBlocks().end()) return false;
         break;
      default:
         assert(0);
         return true;
   }
   loc.trusted = true;
   return true;

}

bool PatchMgr::consistency() const {
   if (!point_maker_) return false;
   if (!instor_) return false;
   if (!as_) return false;
   return (as_->consistency(this));
}

