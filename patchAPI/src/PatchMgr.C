/* Public Interface */

#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"
#include "Point.h"
#include "PatchCallback.h"

using namespace Dyninst;
using namespace PatchAPI;

using Dyninst::ParseAPI::CodeSource;
using Dyninst::InstructionAPI::InstructionDecoder;
using Dyninst::InstructionAPI::Instruction;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::PointMakerPtr;
using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PointSet;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchFunction;

bool debug_patchapi_flag = false;

static void
initDebugFlag() {
  if (getenv("PATCHAPI_DEBUG"))
    debug_patchapi_flag = true;
}

PatchMgr::PatchMgr(AddrSpacePtr as, PointMakerPtr pt, 
                   InstrumenterPtr inst)
  : point_maker_(pt), as_(as) {
  if (inst == InstrumenterPtr()) {
    instor_ = Instrumenter::create(as);
  } else {
    inst->setAs(as);
    instor_ = inst;
  }
}

PatchMgrPtr
PatchMgr::create(AddrSpacePtr as, PointMakerPtr pf, InstrumenterPtr inst) {
   PatchMgrPtr ret = PatchMgrPtr(new PatchMgr(as, pf, inst));
  if (!ret) return PatchMgrPtr();
  initDebugFlag();
  ret->as_->mgr_ = ret;
  ret->pointMaker()->setMgr(ret);
  patch_cerr << "PatchAPI starts.\n";
  patch_cerr << ws2 << "Glue Instrumenter and Linker ot PatchMgr.\n";
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
   // TODO: do we delete PatchObjects, etc?
}

template <class A>
struct SplitPredicate {
   bool operator()(const Point::Type &t, const Location &l, A a) {
      // Check block
      if (l.block != a.first) return false;
      // Exit points get moved
      if (t == Point::BlockExit) return true;
      // Check address
      if ((t || Point::InsnTypes) &&
          (l.addr >= a.first->end())) return true;
      return false;
   }
};

void PatchMgr::updatePointsForBlockSplit(PatchBlock *oldBlock, PatchBlock *newBlock) {
   SplitPredicate<std::pair<PatchBlock *, PatchBlock *> > pred;

   std::pair<PatchBlock *, PatchBlock *> blocks(oldBlock, newBlock);
   // Move any Points that were in the old block and should be in the new block over.
   std::set<Point *> points;
   findPoints(Scope(oldBlock),
              Point::BlockExit | Point::InsnTypes,
              pred,
              blocks,
              std::inserter(points, points.begin()),
              false); 

   std::vector<PatchFunction *> funcs; 
   oldBlock->getFunctions(std::back_inserter(funcs));
   for (std::vector<PatchFunction *>::iterator tmp = funcs.begin();
        tmp != funcs.end(); ++tmp) {
      std::vector<Point *> funcPoints;
      findPoints(Scope(*tmp, oldBlock),
                 Point::BlockExit | Point::InsnTypes,
                 pred,
                 blocks,
                 std::inserter(points, points.begin()),
                 false); 
   }

   for (std::set<Point *>::iterator iter = points.begin();
        iter != points.end(); ++iter) {
      (*iter)->changeBlock(newBlock);
   }
   
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
   return ((types | Point::FuncDuring || types | Point::FuncEntry) &&
           (scope.func || scope.obj || scope.wholeProgram));
}

bool PatchMgr::wantCallSites(Scope &scope, Point::Type types) {
   return ((types | Point::CallTypes) &&
           (scope.func || scope.obj || scope.wholeProgram));
}

bool PatchMgr::wantExitSites(Scope &scope, Point::Type types) {
   return ((types | Point::FuncExit) &&
           (scope.func || scope.obj || scope.wholeProgram));
}

bool PatchMgr::wantBlocks(Scope &scope, Point::Type types) {
   return (scope.func == NULL && types | Point::BlockTypes);
}

bool PatchMgr::wantBlockInstances(Scope &scope, Point::Type types) {
   return (scope.func != NULL && types | Point::BlockTypes);
}

bool PatchMgr::wantEdges(Scope &scope, Point::Type types) {
   return (scope.func == NULL && types | Point::EdgeTypes);
}

bool PatchMgr::wantInsns(Scope &scope, Point::Type types) {
   return (scope.func == NULL && types | Point::InsnTypes);
}

bool PatchMgr::wantInsnInstances(Scope &scope, Point::Type types) {
   return (scope.func == NULL && types | Point::InsnTypes);
}

void PatchMgr::getFuncCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   // We can either have a scope of PatchObject and be looking for all
   // the functions it contains, a scope of a Function, or be looking
   // for every single function we know about. 
   Functions funcs;
   getFuncs(scope, funcs);

   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      if (types & Point::FuncDuring) ret.push_back(Candidate(Location(*iter), Point::FuncDuring));
      if (types & Point::FuncEntry) ret.push_back(Candidate(Location(*iter, (*iter)->entry()), Point::FuncEntry));
   }
}


void PatchMgr::getCallSiteCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   CallSites sites;
   getCallSites(scope, sites);
   for (CallSites::iterator iter = sites.begin(); iter != sites.end(); ++iter) {
      if (types & Point::PreCall) ret.push_back(Candidate(Location(*iter), Point::PreCall));
      if (types & Point::PostCall) ret.push_back(Candidate(Location(*iter), Point::PostCall));
   }
}

void PatchMgr::getExitSiteCandidates(Scope &scope, Point::Type, Candidates &ret) {
   ExitSites sites;
   getExitSites(scope, sites);
   for (ExitSites::iterator iter = sites.begin(); iter != sites.end(); ++iter) {
      ret.push_back(Candidate(Location(*iter), Point::FuncExit));
   }
}

void PatchMgr::getBlockCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   Blocks blocks;
   getBlocks(scope, blocks);
   for (Blocks::iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      if (types & Point::BlockEntry) ret.push_back(Candidate(Location(*iter), Point::BlockEntry));
      if (types & Point::BlockDuring) ret.push_back(Candidate(Location(*iter), Point::BlockDuring));
      if (types & Point::BlockExit) ret.push_back(Candidate(Location(*iter), Point::BlockExit));
   }
}

void PatchMgr::getEdgeCandidates(Scope &scope, Point::Type, Candidates &ret) {
   Edges edges;
   getEdges(scope, edges);
   for (Edges::iterator iter = edges.begin(); iter != edges.end(); ++iter) {
      ret.push_back(Candidate(Location(*iter), Point::EdgeDuring));
   }
}

void PatchMgr::getInsnCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   Insns insns;
   getInsns(scope, insns);
   for (Insns::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      if (types & Point::PreInsn) ret.push_back(Candidate(Location(*iter), Point::PreInsn));
      if (types & Point::PostInsn) ret.push_back(Candidate(Location(*iter), Point::PostInsn));
   }
}

void PatchMgr::getBlockInstanceCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   BlockInstances blocks;
   getBlockInstances(scope, blocks);
   for (BlockInstances::iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      if (types & Point::BlockEntry) ret.push_back(Candidate(Location(iter->first, iter->second), Point::BlockEntry));
      if (types & Point::BlockDuring) ret.push_back(Candidate(Location(iter->first, iter->second), Point::BlockDuring));
      if (types & Point::BlockExit) ret.push_back(Candidate(Location(iter->first, iter->second), Point::BlockExit));
   }
}

void PatchMgr::getInsnInstanceCandidates(Scope &scope, Point::Type types, Candidates &ret) {
   InsnInstances insns;
   getInsnInstances(scope, insns);
   for (InsnInstances::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      if (types & Point::PreInsn) ret.push_back(Candidate(Location(iter->first, iter->second), Point::PreInsn));
      if (types & Point::PostInsn) ret.push_back(Candidate(Location(iter->first, iter->second), Point::PostInsn));
   }
}

void PatchMgr::getFuncs(Scope &scope, Functions &funcs) {
   if (scope.wholeProgram) {
      AddrSpace::ObjSet &objs = as()->objSet();
      for (AddrSpace::ObjSet::iterator iter = objs.begin(); iter != objs.end(); ++iter) {
         (*iter)->funcs(std::back_inserter(funcs));
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
      const PatchFunction::blockset &c = (*iter)->calls();
      for (PatchFunction::blockset::const_iterator iter2 = c.begin(); iter2 != c.end(); ++iter2) {
         if (!scope.block || (scope.block == *iter2))
            sites.push_back(CallSite(*iter, *iter2));
      }
   }
}

void PatchMgr::getExitSites(Scope &scope, ExitSites &sites) {
   // All sites in whatever functions we want
   Functions funcs;
   getFuncs(scope, funcs);
   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      const PatchFunction::blockset &e = (*iter)->exits();
      for (PatchFunction::blockset::const_iterator iter2 = e.begin(); iter2 != e.end(); ++iter2) {
         if (!scope.block || (scope.block == *iter2))
            sites.push_back(ExitSite(*iter, *iter2));
      }
   }
}

void PatchMgr::getBlocks(Scope &scope, Blocks &blocks) {
   if (scope.wholeProgram) {
      const AddrSpace::ObjSet &objs = as()->objSet();
      for (AddrSpace::ObjSet::const_iterator iter = objs.begin(); iter != objs.end(); ++iter) {
         (*iter)->blocks(std::back_inserter(blocks));
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
      const AddrSpace::ObjSet &objs = as()->objSet();
      for (AddrSpace::ObjSet::const_iterator iter = objs.begin(); iter != objs.end(); ++iter) {
         (*iter)->edges(std::back_inserter(edges));
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
         insns.push_back(InsnLoc(*iter, t->first, t->second));
      }
   }
}

void PatchMgr::getBlockInstances(Scope &scope, BlockInstances &blocks) {
   Functions funcs;
   getFuncs(scope, funcs);
   
   for (Functions::iterator iter = funcs.begin(); iter != funcs.end(); ++iter) {
      const PatchFunction::Blockset &b = (*iter)->blocks();
      for (PatchFunction::Blockset::const_iterator iter2 = b.begin(); iter2 != b.end(); ++iter2) {
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
         PatchBlock::Insns i;
         (*iter2)->getInsns(i);
         for (PatchBlock::Insns::iterator iter3 = i.begin(); iter3 != i.end(); ++iter3) {
            insns.push_back(InsnInstance(*iter, InsnLoc(*iter2, iter3->first, iter3->second)));
         }
      }
   }
}


void PatchMgr::enumerateTypes(Point::Type types, EnumeratedTypes &out) {
   for (unsigned i = 0; i <= 31; ++i) {
      Point::Type tmp = (Point::Type) type_val(i);
      if (types & tmp) out.push_back(tmp);
   }
};


void PatchMgr::destroy(Point *p) {
   p->obj()->cb()->destroy(p);
}
