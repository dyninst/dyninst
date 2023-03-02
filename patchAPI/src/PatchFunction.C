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

#include "PatchCFG.h"
#include "PatchMgr.h"
#include "PatchCallback.h"

#include "CFG.h"

using namespace std;
using namespace Dyninst;
using namespace PatchAPI;

PatchFunction*
PatchFunction::create(ParseAPI::Function *f, PatchObject* obj) {
  return obj->getFunc(f);
}

PatchFunction::PatchFunction(ParseAPI::Function *f,
                             PatchObject* o) : 
   func_(f), obj_(o), addr_((obj_->codeBase() + func_->addr()) & obj_->addrMask()),
   _loop_analyzed(false), _loop_root(NULL),
   isDominatorInfoReady(false),	isPostDominatorInfoReady(false)
{
}

PatchFunction::PatchFunction(const PatchFunction *parFunc, PatchObject* child)
  : func_(parFunc->func_), obj_(child), addr_(obj_->codeBase() + func_->addr()),
    _loop_analyzed(false), _loop_root(NULL),
   isDominatorInfoReady(false),	isPostDominatorInfoReady(false)
{
}


const PatchFunction::Blockset&
PatchFunction::blocks() {
  if (all_blocks_.size() == func_->num_blocks()) 
      return all_blocks_;

  if (!all_blocks_.empty()) { // recompute other block lists if block list grew
      if (!call_blocks_.empty()) call_blocks_.clear();
      if (!exit_blocks_.empty()) exit_blocks_.clear();
  }

  // Otherwise we need to create them
  for (ParseAPI::Function::blocklist::iterator iter = func_->blocks().begin();
       iter != func_->blocks().end(); ++iter) {
    all_blocks_.insert(obj()->getBlock(*iter));
  }
  return all_blocks_;
}

PatchBlock*
PatchFunction::entry() {
  assert(obj());
  assert(func_);

  ParseAPI::Block* ientry = func_->entry();
  if (!ientry) {
    // In case we haven't parsed yet ...
     blocks();
    ientry = func_->entry();
  }
  assert(ientry);
  return obj()->getBlock(ientry);
}

const PatchFunction::Blockset&
PatchFunction::exitBlocks() {
  for (auto iter = func_->exitBlocks().begin();
       iter != func_->exitBlocks().end(); ++iter) {
    PatchBlock* pblk = obj()->getBlock(*iter);
    exit_blocks_.insert(pblk);
  }
  return exit_blocks_;
}

const PatchFunction::Blockset&
PatchFunction::callBlocks() {
  // Compute the list if it's empty or if the list of function blocks
  // has grown
  if (call_blocks_.empty() && !func_->callEdges().empty())
  {
    const ParseAPI::Function::edgelist &callEdges = func_->callEdges();
    for (ParseAPI::Function::edgelist::iterator iter = callEdges.begin();
         iter != callEdges.end(); ++iter) {
      ParseAPI::Block *src = (*iter)->src();
      PatchBlock *block = obj()->getBlock(src);
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

   // pull all of b's points from blockPoints_
   destroyBlockPoints(b);
   cb()->remove_block(this, b);
}

static bool hasSingleIndirectSinkEdge(PatchBlock *b)
{
   fprintf(stderr,"hasSingleIndirectSinkEdge(%lx)=",b->start());
   const ParseAPI::Block::edgelist & trgs = b->block()->targets();
   if (trgs.size() == 1) {
      ParseAPI::Edge *edge = * trgs.begin();
      if (edge->sinkEdge() && edge->type() == ParseAPI::INDIRECT) {
         fprintf(stderr,"true\n");
         return true;
      }
   }
   fprintf(stderr,"false\n");
   return false;
}

void PatchFunction::addBlock(PatchBlock *b) {
   if (all_blocks_.empty() && exit_blocks_.empty() && call_blocks_.empty()) return;

   all_blocks_.insert(b);

   if (!call_blocks_.empty()) {
      if (b->containsCall()) {
         call_blocks_.insert(b);
      } 
      else if (hasSingleIndirectSinkEdge(b)) {
         // don't know what the edge will resolve to, until then the 
         // call_blocks_ vector shouldn't be considered complete
         call_blocks_.clear(); 
      }
   }

   if (0 < b->numRetEdges() && !exit_blocks_.empty()) {
      exit_blocks_.insert(b);
   }

   cb()->add_block(this, b);
}
   
Point *PatchFunction::findPoint(Location loc, Point::Type type, bool create) {
   PointMaker* maker = obj_->addrSpace()->mgr()->pointMaker();
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
         iter = blockPoints_.insert(blockPoints_.begin(), std::make_pair(loc.block, std::move(bp)));
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
            if (!loc.addr || !loc.insn.isValid()) {
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
            if (!loc.addr || !loc.insn.isValid()) return NULL;
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
         iter = edgePoints_.insert(edgePoints_.begin(), std::make_pair(loc.edge, std::move(ep)));
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
            if (create && !loc.trusted && !verifyExit(loc.block)) return NULL;
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
            if (create && !loc.trusted && !verifyCall(loc.block)) return NULL;
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
            if (create && !loc.trusted && !verifyCall(loc.block)) return NULL;
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
}

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
   else 
      return false;
}

// remove block points from points_ and blockPoints_
void PatchFunction::destroyBlockPoints(PatchBlock *block)
{
    PatchCallback *cb = obj()->cb();

    // remove from points_
    if (points_.entry && points_.entry->block() == block) {
        cb->destroy(points_.entry); // KEVINTEST: can you delete a function's entry point?
        points_.entry = NULL;
    }
    std::map<PatchBlock *, Point *>::iterator pit = points_.exits.find(block);
    if (pit != points_.exits.end()) {
        cb->destroy(pit->second);
        points_.exits.erase(pit);
    }
    pit = points_.postCalls.find(block);
    if (pit != points_.postCalls.end()) {
        cb->destroy(pit->second);
        points_.postCalls.erase(pit);
    }
    pit = points_.preCalls.find(block);
    if (pit != points_.preCalls.end()) {
        cb->destroy(pit->second);
        points_.preCalls.erase(pit);
    }
    
    // remove from blockPoints_
    map<PatchBlock *, BlockPoints>::iterator bit = blockPoints_.find(block);
    if (bit == blockPoints_.end()) {
        return;
    }
    if (bit->second.during) {
        bit->first->remove(bit->second.during);
        cb->destroy(bit->second.during);
        bit->second.during = NULL;
    }
    if (bit->second.entry) {
        bit->first->remove(bit->second.entry);
        cb->destroy(bit->second.entry);
        bit->second.entry = NULL;
    }
    if (bit->second.exit) {
        bit->first->remove(bit->second.exit);
        cb->destroy(bit->second.exit);
        bit->second.exit = NULL;
    }
    if (!bit->second.postInsn.empty()) {
        for (InsnPoints::iterator iit = bit->second.postInsn.begin();
             iit != bit->second.postInsn.end();
             iit++)
        {
            bit->first->remove(iit->second);
            cb->destroy(iit->second);
        }
        bit->second.postInsn.clear();
    }
    if (!bit->second.preInsn.empty()) {
        for (InsnPoints::iterator iit = bit->second.preInsn.begin();
             iit != bit->second.preInsn.end();
             iit++)
        {
            bit->first->remove(iit->second);
            cb->destroy(iit->second);
        }
        bit->second.preInsn.clear();
    }
    blockPoints_.erase(bit);
}

void PatchFunction::destroyPoints() 
{
    PatchCallback *cb = obj()->cb();
    // 1) clear blockPoints_ (remove from block as well)
    // 2) clear edgePoints_ (remove from edge as well)
    // 3) clear points_

    // 1)
    map<PatchBlock *, BlockPoints>::iterator bit = blockPoints_.begin();
    while (bit != blockPoints_.end()) 
    {
        destroyBlockPoints(bit->first); // eliminates bit from blockPoints_
        bit = blockPoints_.begin();
    }
    blockPoints_.clear();

    // 2)
    for(map<PatchEdge *, EdgePoints>::iterator eit = edgePoints_.begin(); 
        eit != edgePoints_.end(); eit++) 
    {
        if (eit->second.during) {
            eit->first->remove(eit->second.during);
            cb->destroy(eit->second.during);
            eit->second.during = NULL;
        }
    }
    edgePoints_.clear();
    
    // 3)
    if (points_.entry) {
       cb->destroy(points_.entry);
       points_.entry = NULL;
    }
    if (points_.during) {
       cb->destroy(points_.during);
       points_.during = NULL;
    }
    if (!points_.exits.empty()) {
        for (map<PatchBlock *, Point *>::iterator pit = points_.exits.begin();
             pit != points_.exits.end();
             pit++)
        {
            cb->destroy(pit->second);
        }
        points_.exits.clear();
    }
    if (!points_.postCalls.empty()) {
        for (map<PatchBlock *, Point *>::iterator pit = points_.postCalls.begin();
             pit != points_.postCalls.end();
             pit++)
        {
            cb->destroy(pit->second);
        }
        points_.postCalls.clear();
    }
    if (!points_.preCalls.empty()) {
        for (map<PatchBlock *, Point *>::iterator pit = points_.preCalls.begin();
             pit != points_.preCalls.end();
             pit++)
        {
            cb->destroy(pit->second);
        }
        points_.preCalls.clear();
    }
}


void PatchFunction::remove(Point *p) {
   assert(p->func() == this);

   switch(p->type()) {
      case Point::PreInsn:
         blockPoints_[p->block()].preInsn.erase(p->addr());
         break;
      case Point::PostInsn:
         blockPoints_[p->block()].postInsn.erase(p->addr());
         break;
      case Point::BlockEntry:
         blockPoints_[p->block()].entry = NULL;
         break;
      case Point::BlockExit:
         blockPoints_[p->block()].exit = NULL;
         break;
      case Point::BlockDuring:
         blockPoints_[p->block()].during = NULL;
         break;
      case Point::FuncEntry:
         points_.entry = NULL;
         break;
      case Point::FuncExit:
         points_.exits.erase(p->block());
         break;
      case Point::FuncDuring:
         points_.during = NULL;
         break;
      case Point::PreCall:
         points_.preCalls.erase(p->block());
         break;
      case Point::PostCall:
         points_.postCalls.erase(p->block());
         break;
      case Point::EdgeDuring: {
         map<PatchEdge *, EdgePoints>::iterator eit = edgePoints_.find(p->edge());
         if (eit != edgePoints_.end()) {
            edgePoints_.erase(p->edge());
         }
         break;
      }
      default:
         assert(0 && "deleting point of unexpected type");
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
   // 3) fix function's funcPoints_ map for preCall, postCall, and exit points
   // 4) if the block has no points, we're done
   // 5) fix block's blockPoints_ map for exit, preInsn, and postInsn points

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
   map<PatchBlock*,Point*>::iterator pit = points_.exits.find(first);
   if (pit != points_.exits.end()) {
       pit->second->changeBlock(second);
       points_.exits[second] = pit->second;
       points_.exits.erase(pit);
   }
   pit = points_.preCalls.find(first);
   if (pit != points_.preCalls.end()) {
       pit->second->changeBlock(second);
       points_.preCalls[second] = pit->second;
       points_.preCalls.erase(pit);
   }
   pit = points_.postCalls.find(first);
   if (pit != points_.postCalls.end()) {
       pit->second->changeBlock(second);
       points_.postCalls[second] = pit->second;
       points_.postCalls.erase(pit);
   }

   // 4)
   std::map<PatchBlock *, BlockPoints>::iterator iter = blockPoints_.find(first);
   if (iter == blockPoints_.end()) 
       return;

   // 5)
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


bool PatchFunction::consistency() const {
   if (!obj_) {
      cerr << "Error: no object!" << endl;
      CONSIST_FAIL;
   }

   if (!all_blocks_.empty()) {
      if (all_blocks_.size() != func_->num_blocks()) {
         cerr << "Error: size mismatch in all_blocks; PatchAPI " << all_blocks_.size() 
              << " and ParseAPI " << func_->num_blocks() << endl;
         CONSIST_FAIL;
      }
      for (Blockset::const_iterator iter = all_blocks_.begin(); iter != all_blocks_.end(); ++iter) {
          if (!(*iter)->consistency()) {
             CONSIST_FAIL;
             cerr << "Error: block ["<< hex << (*iter)->start() << " " 
                 << (*iter)->end() << dec << ") failed consistency check" << endl;
          }
         bool found = false;
         for (ParseAPI::Function::blocklist::iterator iter2 = func_->blocks().begin();
              iter2 != func_->blocks().end(); ++iter2) {
            if ((*iter)->block() == *iter2) {
               found = true;
               break;
            }
         }
         if (!found) {
            cerr << "Error: found block not in ParseAPI" << endl;
            CONSIST_FAIL;
         }
      }
   }

   if (!exit_blocks_.empty()) {
      for (Blockset::const_iterator iter = exit_blocks_.begin(); iter != exit_blocks_.end(); ++iter) {
         bool found = false;
         // should compare exit blocks not return blocks (fixed).
         for (auto iter2 = func_->exitBlocks().begin();
              iter2 != func_->exitBlocks().end(); ++iter2) {
            if ((*iter)->block() == *iter2) {
               found = true;
               break;
            }
         }
         if (!found) {
            cerr << "Error: found exit block not in ParseAPI" << endl;
            CONSIST_FAIL;
         }
      }
   }

   if (!call_blocks_.empty()) {
      // build up set of parseAPI-level call blocks
      set<ParseAPI::Block*> llcbs;
      for (ParseAPI::Function::edgelist::iterator llit = func_->callEdges().begin();
           llit != func_->callEdges().end(); ++llit) 
      {
          llcbs.insert((*llit)->src());
      }
      if (call_blocks_.size() != llcbs.size()) {
         cerr << "PatchAPI call_blocks_ not same size ("<<call_blocks_.size()
            <<") as ParseAPI call blocks list ("<<llcbs.size()<<")"<<endl; 
         //CONSIST_FAIL;
      }
      // verify that call_blocks_ are in llcbs
      for (Blockset::const_iterator cit = call_blocks_.begin(); 
           cit != call_blocks_.end(); ++cit) 
      {
          if (llcbs.find((*cit)->block()) == llcbs.end()) {
              CONSIST_FAIL;
          }
      }
      // don't need to check that llcbs are in call_blocks_ since we've 
      // verified the sets are the same size
   }

   if (!points_.consistency(this))  {
      cerr << "Error: failed point consistency" << endl;
      CONSIST_FAIL;
   }

   for (std::map<PatchBlock *, BlockPoints>::const_iterator iter = blockPoints_.begin();
        iter != blockPoints_.end(); ++iter) {
      if (!(iter->second.consistency(iter->first, this))) {
         cerr << "Error: failed block point consistency" << endl;
         CONSIST_FAIL;
      }
      if (all_blocks_.find(iter->first) == all_blocks_.end()) {
         cerr << "Error: points for a block not in the function" << endl;
         CONSIST_FAIL;
      }
   }

   for (std::map<PatchEdge *, EdgePoints>::const_iterator iter = edgePoints_.begin();
        iter != edgePoints_.end(); ++iter) {
      if (!iter->second.consistency(iter->first, this)) 
          CONSIST_FAIL;
   }

   return true;
}


bool FuncPoints::consistency(const PatchFunction *func) const {
   if (entry) {
      if (entry->type() != Point::FuncEntry) {
         cerr << "Error: entry point has wrong type" << endl;
         CONSIST_FAIL;
      }
      if (!entry->consistency()) {
         cerr << "Error: entry point inconsistent" << endl; 
         CONSIST_FAIL;
      }
      if (entry->func() != func) {
         cerr << "Error: entry point has wrong func" << endl;
         CONSIST_FAIL;
      }
   }
   if (during) {
      if (during->type() != Point::FuncDuring) {
         cerr << "Error: during point has wrong type" << endl;
         CONSIST_FAIL;
      }
      if (!during->consistency()) {
         cerr << "Error: during point inconsistent" << endl; 
         CONSIST_FAIL;
      }
      if (during->func() != func) {
         cerr << "Error: during point has wrong func" << endl;
         CONSIST_FAIL;
      }
   }
   for (std::map<PatchBlock *, Point *>::const_iterator iter = exits.begin();
        iter != exits.end(); ++iter) {
      if (iter->second->type() != Point::FuncExit) {
         cerr << "Error: exit point has non-exit type" << endl;
         CONSIST_FAIL;
      }
      if (!iter->second->consistency()) {
         cerr << "Error: exit point inconsistent" << endl;
         CONSIST_FAIL;
      }
      if (iter->second->func() != func) {
         cerr << "Error: exit point has a different func" << endl;
         CONSIST_FAIL;
      }
      if (iter->second->block() != iter->first) {
         cerr << "Error: exit point has incorrect block" << endl;
         CONSIST_FAIL;
      }
      if (!func->verifyExitConst(iter->first)) {
         cerr << "Error: exit point has non-exit block" << endl;
         CONSIST_FAIL;
      }
   }
   for (std::map<PatchBlock *, Point *>::const_iterator iter = preCalls.begin();
        iter != preCalls.end(); ++iter) {
      if (iter->second->type() != Point::PreCall) {
         cerr << "Error: preCall point has wrong type" << endl;
         CONSIST_FAIL;
      }
      if (!iter->second->consistency()) {
         cerr << "Error: preCall point inconsistent" << endl;
         CONSIST_FAIL;
      }
      if (iter->second->func() != func) {
         cerr << "Error: preCall point has wrong function" << endl;
         CONSIST_FAIL;
      }
      if (iter->second->block() != iter->first) {
         cerr << "Error: preCall point has wrong block" << endl;
         CONSIST_FAIL;
      }
      if (!func->verifyCallConst(iter->first)) {
         cerr << "Error: preCall point has non-call block" << endl;
         CONSIST_FAIL;
      }
   }
   for (std::map<PatchBlock *, Point *>::const_iterator iter = postCalls.begin();
        iter != postCalls.end(); ++iter) {
      if (iter->second->type() != Point::PostCall) {
         cerr << "Error: postCall point has wrong type" << endl;
         CONSIST_FAIL;
      }
      if (!iter->second->consistency()) {
         cerr << "Error: postCall point inconsistent" << endl;
         CONSIST_FAIL;
      }
      if (iter->second->func() != func) {
         cerr << "Error: postCall function incorrect" << endl;
         CONSIST_FAIL;
      }
      if (iter->second->block() != iter->first) {
         cerr << "Error: postCall block incorrect" << endl;
         CONSIST_FAIL;
      }
      if (!func->verifyCallConst(iter->first)) {
         cerr << "Error: postCall has non-call block" << endl;
         CONSIST_FAIL;
      }
   }
   return true;
}

void PatchFunction::invalidateBlocks() {
   all_blocks_.clear();
   call_blocks_.clear();
   return_blocks_.clear();
   exit_blocks_.clear();
}

PatchLoopTreeNode* PatchFunction::getLoopTree() {
  if (_loop_root == NULL) {
      if (_loop_analyzed == false) {
          createLoops();
	  _loop_analyzed = true;
      }
      _loop_root = new PatchLoopTreeNode(obj_, func_->getLoopTree(), _loop_map);
      
  }
  return _loop_root;
}

// this methods returns the loop objects that exist in the control flow
// grap. It returns a set. And if there are no loops, then it returns the empty
// set. not NULL.
void PatchFunction::getLoopsByNestingLevel(vector<PatchLoop*>& lbb, bool outerMostOnly)
{
  if (_loop_analyzed == false) {
      createLoops();
      _loop_analyzed = true;
  }

  for (std::set<PatchLoop *>::iterator iter = _loops.begin();
       iter != _loops.end(); ++iter) {
     // if we are only getting the outermost loops
     if (outerMostOnly && 
         (*iter)->parent != NULL) continue;

     lbb.push_back(*iter);
  }
  return;
}


// get all the loops in this flow graph
bool PatchFunction::getLoops(vector<PatchLoop*>& lbb)
{
  getLoopsByNestingLevel(lbb, false);
  return true;
}

// get the outermost loops in this flow graph
bool PatchFunction::getOuterLoops(vector<PatchLoop*>& lbb)
{
  getLoopsByNestingLevel(lbb, true);
  return true;
}
PatchLoop *PatchFunction::findLoop(const char *name)
{
  return getLoopTree()->findLoop(name);
}

void PatchFunction::createLoops() {
    vector<ParseAPI::Loop*> loops;    
    func_->getLoops(loops);
    
    // Create all the PatchLoop objects in the function
    for (auto lit = loops.begin(); lit != loops.end(); ++lit) {
        PatchLoop* pl = new PatchLoop(obj_, *lit);
	_loop_map[*lit] = pl;
        _loops.insert(pl);
    }

    // Build nesting relations among loops
    for (auto lit = loops.begin(); lit != loops.end(); ++lit) {
         ParseAPI::Loop* l = *lit;
         PatchLoop *pl = _loop_map[l];
	 // set parent pointer
         if (l->parentLoop() != NULL)
	     pl->parent = _loop_map[l->parentLoop()];
	 // set contained loop vector
         vector<ParseAPI::Loop*> containedLoops;
	 l->getContainedLoops(containedLoops);
	 for (auto lit2 = containedLoops.begin(); lit2 != containedLoops.end(); ++lit2)
	     pl->containedLoops.insert(_loop_map[*lit2]);
    }     
}

void PatchFunction::fillDominatorInfo()
{
    if (!isDominatorInfoReady) {
        // Fill immediate dominator info
	for (auto bit = blocks().begin(); bit != blocks().end(); ++bit) {
	    ParseAPI::Block* b = (*bit)->block();
	    ParseAPI::Block* imd = func_->getImmediateDominator(b);
	    if (imd == NULL)
	        immediateDominator[*bit] = NULL;
	    else
	        immediateDominator[*bit] = obj_->getBlock(imd);
	}
	// Fill immediate dominates info
	for (auto bit = blocks().begin(); bit != blocks().end(); ++bit) {
	    ParseAPI::Block* b = (*bit)->block();
	    set<ParseAPI::Block*> dominates;
	    func_->getImmediateDominates(b, dominates);
	    immediateDominates[*bit] = new set<PatchBlock*>;
	    for (auto dit = dominates.begin(); dit != dominates.end(); ++dit)
	        immediateDominates[*bit]->insert(obj_->getBlock(*dit));
	}
	isDominatorInfoReady = true;
    }
}

void PatchFunction::fillPostDominatorInfo()
{
    if (!isPostDominatorInfoReady) {
        // Fill immediate post-dominator info
	for (auto bit = blocks().begin(); bit != blocks().end(); ++bit) {
	    ParseAPI::Block* b = (*bit)->block();
	    ParseAPI::Block* imd = func_->getImmediatePostDominator(b);
	    if (imd == NULL)
	        immediatePostDominator[*bit] = NULL;
	    else
	        immediatePostDominator[*bit] = obj_->getBlock(imd);
	}
	// Fill immediate post-dominates info
	for (auto bit = blocks().begin(); bit != blocks().end(); ++bit) {
	    ParseAPI::Block* b = (*bit)->block();
	    set<ParseAPI::Block*> postDominates;
	    func_->getImmediatePostDominates(b, postDominates);
	    immediatePostDominates[*bit] = new set<PatchBlock*>;
	    for (auto dit = postDominates.begin(); dit != postDominates.end(); ++dit)
	        immediatePostDominates[*bit]->insert(obj_->getBlock(*dit));
	}
	isPostDominatorInfoReady = true;
    }
}

bool PatchFunction::dominates(PatchBlock* A, PatchBlock *B) {
    if (A == NULL || B == NULL) return false;
    if (A == B) return true;

    fillDominatorInfo();

    if (!immediateDominates[A]) return false;

    for (auto bit = immediateDominates[A]->begin(); bit != immediateDominates[A]->end(); ++bit)
        if (dominates(*bit, B)) return true;
    return false;
}
        
PatchBlock* PatchFunction::getImmediateDominator(PatchBlock *A) {
    fillDominatorInfo();
    return immediateDominator[A];
}

void PatchFunction::getImmediateDominates(PatchBlock *A, set<PatchBlock*> &imd) {
    fillDominatorInfo();
    if (immediateDominates[A] != NULL)
        imd.insert(immediateDominates[A]->begin(), immediateDominates[A]->end());
}

void PatchFunction::getAllDominates(PatchBlock *A, set<PatchBlock*> &d) {
    fillDominatorInfo();
    d.insert(A);
    if (immediateDominates[A] == NULL) return;

    for (auto bit = immediateDominates[A]->begin(); bit != immediateDominates[A]->end(); ++bit)
        getAllDominates(*bit, d);
}

bool PatchFunction::postDominates(PatchBlock* A, PatchBlock *B) {
    if (A == NULL || B == NULL) return false;
    if (A == B) return true;

    fillPostDominatorInfo();

    if (!immediatePostDominates[A]) return false;

    for (auto bit = immediatePostDominates[A]->begin(); bit != immediatePostDominates[A]->end(); ++bit)
        if (postDominates(*bit, B)) return true;
    return false;
}
        
PatchBlock* PatchFunction::getImmediatePostDominator(PatchBlock *A) {
    fillPostDominatorInfo();
    return immediatePostDominator[A];
}

void PatchFunction::getImmediatePostDominates(PatchBlock *A, set<PatchBlock*> &imd) {
    fillPostDominatorInfo();
    if (immediatePostDominates[A] != NULL)
        imd.insert(immediatePostDominates[A]->begin(), immediatePostDominates[A]->end());
}

void PatchFunction::getAllPostDominates(PatchBlock *A, set<PatchBlock*> &d) {
    fillPostDominatorInfo();
    d.insert(A);
    if (immediatePostDominates[A] == NULL) return;

    for (auto bit = immediatePostDominates[A]->begin(); bit != immediatePostDominates[A]->end(); ++bit)
        getAllPostDominates(*bit, d);
}
