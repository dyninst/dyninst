/* Public Interface */

#include "PatchCFG.h"
#include "PatchMgr.h"
#include "PatchCallback.h"

using namespace Dyninst;
using namespace PatchAPI;

PatchFunction*
PatchFunction::create(ParseAPI::Function *f, PatchObject* obj) {
  return obj->getFunc(f);
}

PatchFunction::PatchFunction(ParseAPI::Function *f,
     PatchObject* o) : func_(f), obj_(o), addr_(obj_->codeBase() + func_->addr()) {}

PatchFunction::PatchFunction(const PatchFunction *parFunc, PatchObject* child)
  : func_(parFunc->func_), obj_(child), addr_(obj_->codeBase() + func_->addr()) {}

const PatchFunction::Blockset&
PatchFunction::getAllBlocks() {
  if (all_blocks_.size() == func_->blocks().size()) 
      return all_blocks_;

  if (!all_blocks_.empty()) { // recompute other block lists if block list grew
      if (!call_blocks_.empty()) call_blocks_.clear();
      if (!exit_blocks_.empty()) exit_blocks_.clear();
  }

  // Otherwise we need to create them
  for (ParseAPI::Function::blocklist::iterator iter = func_->blocks().begin();
       iter != func_->blocks().end(); ++iter) {
    all_blocks_.insert(object()->getBlock(*iter));
  }
  return all_blocks_;
}

PatchBlock*
PatchFunction::getEntryBlock() {
  assert(object());
  assert(func_);

  ParseAPI::Block* ientry = func_->entry();
  if (!ientry) {
    // In case we haven't parsed yet ...
    getAllBlocks();
    ientry = func_->entry();
  }
  assert(ientry);
  return object()->getBlock(ientry);
}

const PatchFunction::Blockset&
PatchFunction::getExitBlocks() {
  if (func_->returnBlocks().size() != exit_blocks_.size()) 
  {
      for (ParseAPI::Function::blocklist::iterator iter = func_->returnBlocks().begin();
           iter != func_->returnBlocks().end(); ++iter) {
        PatchBlock* pblk = object()->getBlock(*iter);
        exit_blocks_.insert(pblk);
      }
  }
  return exit_blocks_;
}

const PatchFunction::Blockset&
PatchFunction::getCallBlocks() {
  // Compute the list if it's empty or if the list of function blocks
  // has grown
  if (call_blocks_.size() != func_->callEdges().size())
  {
    const ParseAPI::Function::edgelist &callEdges = func_->callEdges();
    for (ParseAPI::Function::edgelist::iterator iter = callEdges.begin();
         iter != callEdges.end(); ++iter) {
      ParseAPI::Block *src = (*iter)->src();
      PatchBlock *block = object()->getBlock(src);
      assert(block);
      call_blocks_.insert(block);
    }
  }
  return call_blocks_;
}

PatchFunction::~PatchFunction() {
}

void PatchFunction::removeBlock(PatchBlock *b) {
   if (all_blocks_.empty() && exit_blocks_.empty() && call_blocks_.empty()) return;

   // Otherwise pull b from all_blocks_, exit_blocks_, and call_blocks_.
   all_blocks_.erase(b);
   exit_blocks_.erase(b);
   call_blocks_.erase(b);

   cb()->remove_block(this, b);
}

void PatchFunction::addBlock(PatchBlock *b) {
   if (all_blocks_.empty() && exit_blocks_.empty() && call_blocks_.empty()) return;

   all_blocks_.insert(b);

   cb()->add_block(this, b);
}
   
Point *PatchFunction::findPoint(Location loc, Point::Type type, bool create) {
   PointMakerPtr maker = obj_->addrSpace()->mgr()->pointMaker();
   PatchMgrPtr mgr = obj_->addrSpace()->mgr();

   if (loc.func != this) {
      return NULL;
   }
   Point *ret = NULL;
   if ((type & Point::BlockTypes) || (type & Point::InsnTypes)) {
      if (!loc.block) return NULL;
      std::map<PatchBlock *, BlockPoints>::iterator iter = blockPoints_.find(loc.block);
      if (iter == blockPoints_.end()) {
         if (!create) return NULL;
         BlockPoints bp;
         iter = blockPoints_.insert(blockPoints_.begin(), std::make_pair(loc.block, bp));
      }
      switch (type) {
         case Point::BlockEntry:
            if (!iter->second.entry && create) {
               iter->second.entry = maker->createPoint(loc, type);
            }
            return iter->second.entry;
            break;
         case Point::BlockExit:
            if (!iter->second.exit && create) {
               iter->second.exit = maker->createPoint(loc, type);
            }
            return iter->second.exit;
            break;
         case Point::BlockDuring:
            if (!iter->second.during && create) {
               iter->second.during = maker->createPoint(loc, type);
            }
            return iter->second.during;
            break;
         case Point::PreInsn: {
            if (!loc.addr || !loc.insn) {
               assert(0);
            }
            InsnPoints::iterator iter2 = iter->second.preInsn.find(loc.addr);
            if (iter2 == iter->second.preInsn.end()) {
               if (!create) return NULL;
               ret = maker->createPoint(loc, type);
               iter->second.preInsn[loc.addr] = ret;
               return ret;
            }
            else {
               return iter2->second;
            }
            break;
         }
         case Point::PostInsn: {
            if (!loc.addr || !loc.insn) return NULL;
            InsnPoints::iterator iter2 = iter->second.postInsn.find(loc.addr);
            if (iter2 == iter->second.postInsn.end()) {
               if (!create) return NULL;
               ret = maker->createPoint(loc, type);
               iter->second.postInsn[loc.addr] = ret;
               return ret;
            }
            else return iter2->second;
            break;
         }
         default:
            return NULL;
      }
   }
   else if (type & Point::EdgeTypes) {
      if (!loc.edge) return NULL;
      std::map<PatchEdge *, EdgePoints>::iterator iter = edgePoints_.find(loc.edge);
      if (iter == edgePoints_.end()) {
         if (!create) return NULL;
         EdgePoints ep;
         iter = edgePoints_.insert(edgePoints_.begin(), std::make_pair(loc.edge, ep));
      }
      if (!iter->second.during && create) {
         iter->second.during = maker->createPoint(loc, type);
      }
      return iter->second.during;
   }
   else {
      switch(type) {
         case Point::FuncEntry:
            if (!points_.entry && create) {
               points_.entry = maker->createPoint(loc, type);
            }
            return points_.entry;
         case Point::FuncDuring:
            if (!points_.during && create) {
               points_.during = maker->createPoint(loc, type);
            }
            return points_.during;
         case Point::FuncExit: {
            if (!loc.block) return NULL;
            if (create && loc.untrusted && !verifyExit(loc.block)) return NULL;
            std::map<PatchBlock *, Point *>::iterator iter = points_.exits.find(loc.block);
            if (iter == points_.exits.end()) {
               if (!create) return NULL;
               ret = maker->createPoint(loc, type);
               points_.exits[loc.block] = ret;
               return ret;
            }
            else return iter->second;
         }
         case Point::PreCall: {
            if (!loc.block) return NULL;
            if (create && loc.untrusted && !verifyCall(loc.block)) return NULL;
            std::map<PatchBlock *, Point *>::iterator iter = points_.preCalls.find(loc.block);
            if (iter == points_.preCalls.end()) {
               if (!create) return NULL;
               ret = maker->createPoint(loc, type);
               points_.preCalls[loc.block] = ret;
               return ret;
            }
            else return iter->second;
         }
         case Point::PostCall: {
            if (!loc.block) return NULL;
            if (create && loc.untrusted && !verifyCall(loc.block)) return NULL;
            std::map<PatchBlock *, Point *>::iterator iter = points_.postCalls.find(loc.block);
            if (iter == points_.postCalls.end()) {
               if (!create) return NULL;
               ret = maker->createPoint(loc, type);
               points_.postCalls[loc.block] = ret;
               return ret;
            }
            else return iter->second;
         }
         default:
            return NULL;
      }
   }
   assert(0); return NULL; // unreachable, but keep compiler happy
};

bool PatchFunction::findInsnPoints(Point::Type type,
                                   PatchBlock *block,
                                   InsnPoints::const_iterator &start,
                                   InsnPoints::const_iterator &end) {
   std::map<PatchBlock *, BlockPoints>::iterator iter = blockPoints_.find(block);
   if (iter == blockPoints_.end()) {
      return false;
   }
   if (type == Point::PreInsn) {
      start = iter->second.preInsn.begin();
      end = iter->second.preInsn.end();
      return (start != end);
   }
   else if (type == Point::PostInsn) {
      start = iter->second.postInsn.begin();
      end = iter->second.postInsn.end();
      return (start != end);
   }
   else return false;
}

void PatchFunction::destroy(Point *p) {
   assert(p->getFunction() == this);

   switch(p->type()) {
      case Point::PreInsn:
         delete blockPoints_[p->getBlock()].preInsn[p->address()];
         break;
      case Point::PostInsn:
         delete blockPoints_[p->getBlock()].postInsn[p->address()];
         break;
      case Point::BlockEntry:
         delete blockPoints_[p->getBlock()].entry;
         break;
      case Point::BlockExit:
         delete blockPoints_[p->getBlock()].exit;
         break;
      case Point::BlockDuring:
         delete blockPoints_[p->getBlock()].during;
         break;
      case Point::FuncEntry:
         delete points_.entry;
         break;
      case Point::FuncExit:
         delete points_.exits[p->getBlock()];
         break;
      case Point::FuncDuring:
         delete points_.during;
         break;
      case Point::PreCall:
         delete points_.preCalls[p->getBlock()];
         break;
      case Point::PostCall:
         delete points_.postCalls[p->getBlock()];
         break;
      default:
         break;
   }
}

PatchCallback *PatchFunction::cb() const {
   return obj_->cb();
}

// the "first" block should already be in the function
void PatchFunction::splitBlock(PatchBlock *first, PatchBlock *second)
{
   // 1) add second block to the function
   // 2) fix function's call, exit Blocksets to include second block
   // 3) if the block has no points, we're done
   // 4) fix function's blockPoints_ map for exit, preInsn, and postInsn points

   // 1)
   addBlock(second);

   // 2)
   Blockset::iterator bit = call_blocks_.find(first);
   if (bit != call_blocks_.end()) {
       call_blocks_.erase(*bit);
       call_blocks_.insert(second);
   }

   bit = exit_blocks_.find(first);
   if (bit != exit_blocks_.end()) {
       exit_blocks_.erase(*bit);
       exit_blocks_.insert(second);
   }

   // 3)
   std::map<PatchBlock *, BlockPoints>::iterator iter = blockPoints_.find(first);
   if (iter == blockPoints_.end()) return;

   // 4)
   BlockPoints &points = iter->second;
   BlockPoints &succ = blockPoints_[second];
   if (points.exit) {      
      succ.exit = points.exit;
      points.exit = NULL;
      succ.exit->changeBlock(second);
   }

   InsnPoints::iterator pre = points.preInsn.lower_bound(second->start());
   for (InsnPoints::iterator tmp = pre; tmp != points.preInsn.end(); ++tmp) {
      tmp->second->changeBlock(second);
      succ.preInsn[tmp->first] = tmp->second;
   }
   points.preInsn.erase(pre, points.preInsn.end());

   InsnPoints::iterator post = points.postInsn.lower_bound(second->start());
   for (InsnPoints::iterator tmp = post; tmp != points.postInsn.end(); ++tmp) {
      tmp->second->changeBlock(second);
      succ.postInsn[tmp->first] = tmp->second;
   }
   points.postInsn.erase(post, points.postInsn.end());
}
