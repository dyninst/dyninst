/* Public Interface */

#include "common.h"
#include "PatchCFG.h"
#include "AddrSpace.h"
#include "PatchObject.h"
#include "PatchMgr.h"
#include "PatchCallback.h"
#include "Point.h"
#include <dyn_detail/boost/shared_ptr.hpp>

using namespace Dyninst;
using namespace PatchAPI;
using namespace InstructionAPI;

PatchBlock*
PatchBlock::create(ParseAPI::Block *ib, PatchFunction *f) {
  return f->obj()->getBlock(ib);
}

PatchBlock::PatchBlock(ParseAPI::Block *blk, PatchObject *obj)
  : block_(blk),   obj_(obj) {
}

PatchBlock::PatchBlock(const PatchBlock *parent, PatchObject *child)
  : block_(parent->block_), obj_(child) {
}

void
PatchBlock::getInsns(Insns &insns) const {
  // Pass through to ParseAPI. They don't have a native interface, so add one.
  Offset off = block_->start();
  const unsigned char *ptr =
    (const unsigned char *)block_->region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionDecoder d(ptr, size(), block_->obj()->cs()->getArch());
  while (off < block_->end()) {
    Instruction::Ptr insn = d.decode();
    insns[off + obj_->codeBase()] = insn;
    off += insn->size();
  }
}

const PatchBlock::edgelist&
PatchBlock::getSources() {
  if (srclist_.empty()) {
    for (ParseAPI::Block::edgelist::iterator iter = block_->sources().begin();
         iter != block_->sources().end(); ++iter) {
      PatchEdge *newEdge = obj_->getEdge(*iter, NULL, this);
      srclist_.push_back(newEdge);
    }
  }
  return srclist_;
}

const PatchBlock::edgelist&
PatchBlock::getTargets() {
  if (trglist_.empty()) {
    for (ParseAPI::Block::edgelist::iterator iter = block_->targets().begin();
         iter != block_->targets().end(); ++iter) {
      PatchEdge *newEdge = obj_->getEdge(*iter, this, NULL);
      assert(newEdge);
      trglist_.push_back(newEdge);
    }
  }
  return trglist_;
}

PatchEdge *PatchBlock::findSource(ParseAPI::EdgeTypeEnum type) {
   getSources();
   for (edgelist::iterator iter = srclist_.begin();
        iter != srclist_.end(); ++iter) {
      if ((*iter)->type() == type) return *iter;
   }
   return NULL;
}

PatchEdge *PatchBlock::findTarget(ParseAPI::EdgeTypeEnum type) {
   getTargets();
   for (edgelist::iterator iter = trglist_.begin();
        iter != trglist_.end(); ++iter) {
      assert(*iter);
      assert((*iter)->edge());
      cerr << "Looking for " << ParseAPI::format(type) << ", found edge with " << ParseAPI::format((*iter)->type()) << endl;
      if ((*iter)->type() == type) return *iter;
   }
   return NULL;
}

void PatchBlock::addSourceEdge(PatchEdge *e, bool addIfEmpty) {
   if (!addIfEmpty && srclist_.empty()) return;

   srclist_.push_back(e);

  cb()->add_edge(this, e, PatchCallback::source);
}

void PatchBlock::addTargetEdge(PatchEdge *e, bool addIfEmpty) {
   assert(e);
   if (!addIfEmpty && trglist_.empty()) return;

   trglist_.push_back(e);
   
   cb()->add_edge(this, e, PatchCallback::target);
}


void
PatchBlock::removeSourceEdge(PatchEdge *e) {
   if (srclist_.empty()) return;

  std::vector<PatchEdge *>::iterator iter;
  if ((iter = std::find(srclist_.begin(), srclist_.end(), e)) != srclist_.end()) {
    srclist_.erase(iter);
  }

  cb()->remove_edge(this, e, PatchCallback::source);
}

void
PatchBlock::removeTargetEdge(PatchEdge *e) {
   if (trglist_.empty()) return;

  std::vector<PatchEdge *>::iterator iter;
  if ((iter = std::find(trglist_.begin(), trglist_.end(), e)) != trglist_.end()) {
     cerr << "Erasing target edge" << endl;
    trglist_.erase(iter);
  }
  cb()->remove_edge(this, e, PatchCallback::target);
}


bool
PatchBlock::isShared() {
  return containingFuncs() > 1;
}

PatchBlock::~PatchBlock() {
#if 0
   // Our predecessor may be deleted...
  for (std::vector<PatchEdge *>::iterator iter = srclist_.begin();
       iter != srclist_.end(); ++iter) {
    PatchBlock* blk = (*iter)->source();
    blk->removeTargetEdge(*iter);
  }
  for (std::vector<PatchEdge *>::iterator iter = trglist_.begin();
       iter != trglist_.end(); ++iter) {
    PatchBlock* blk = (*iter)->target();
    blk->removeSourceEdge(*iter);
  }
#endif
}

Address
PatchBlock::start() const {
  return object()->codeBase() + block_->start();
}

Address
PatchBlock::end() const {
  return object()->codeBase() + block_->end();
}

Address
PatchBlock::last() const {
  return object()->codeBase() + block_->lastInsnAddr();
}

Address
PatchBlock::size() const {
  return block_->size();
}

int
PatchBlock::containingFuncs() const {
  return block_->containingFuncs();
}

bool
PatchBlock::containsCall() {
  ParseAPI::Block::edgelist & out_edges = block_->targets();
  ParseAPI::Block::edgelist::iterator eit = out_edges.begin();
  for( ; eit != out_edges.end(); ++eit) {
    if ( ParseAPI::CALL == (*eit)->type() ) {
      return true;
    }
  }
  return false;
}

bool
PatchBlock::containsDynamicCall() {
  ParseAPI::Block::edgelist & out_edges = block_->targets();
  ParseAPI::Block::edgelist::iterator eit = out_edges.begin();
   for( ; eit != out_edges.end(); ++eit) {
     if ( ParseAPI::CALL == (*eit)->type() ) { 
         // see if it's a static call to a bad address
         if ((*eit)->sinkEdge()) {
             using namespace InstructionAPI;
             Instruction::Ptr insn = getInsn(last());
             if (insn->readsMemory()) { // memory indirect
                 return true;
             } else { // check for register indirect
                 set<InstructionAST::Ptr> regs;
                 Expression::Ptr tExpr = insn->getControlFlowTarget();
                 tExpr->getUses(regs);
                 for (set<InstructionAST::Ptr>::iterator rit = regs.begin(); 
                      rit != regs.end(); rit++)
                 {
                     if (RegisterAST::makePC(obj()->co()->cs()->getArch()).getID() != 
                         dyn_detail::boost::dynamic_pointer_cast<RegisterAST>(*rit)->getID()) 
                     {
                         return true;
                     }
                 }
             }
         }
      }
   }
   return false;
}

std::string
PatchBlock::disassemble() const {
  stringstream ret;
  Insns instances;
  getInsns(instances);
  for (Insns::iterator iter = instances.begin();
       iter != instances.end(); ++iter) {
    ret << "\t" << hex << iter->first << ": " << iter->second->format() << dec << endl;
  }
  return ret.str();
}

InstructionAPI::Instruction::Ptr
PatchBlock::getInsn(Address a) const {
   Insns insns;
   getInsns(insns);
   return insns[a];
}

std::string
PatchBlock::long_format() const {
  stringstream ret;
  ret << format() << endl;
  
  Insns insns;
  getInsns(insns);
  
  for (Insns::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
     ret << "\t" << hex << iter->first << " : " << iter->second->format() << dec << endl;
  }
  return ret.str();
}

std::string
PatchBlock::format() const {
  stringstream ret;
  ret << "B[" << hex << start() << "," << end() << "]" << dec;

  return ret.str();
}

PatchFunction*
PatchBlock::getFunction(ParseAPI::Function* f) {
  return obj_->getFunc(f);
}

ParseAPI::Block*
PatchBlock::block() const { return block_; }

PatchObject*
PatchBlock::object() const { return obj_; }

PatchFunction*
PatchBlock::getCallee() {
  PatchBlock::edgelist::const_iterator it = getTargets().begin();
  for (; it != getTargets().end(); ++it) {
    if ((*it)->type() == ParseAPI::CALL) {
      PatchBlock* trg = (*it)->target();
      return obj_->getFunc(obj_->co()->findFuncByEntry(trg->block()->region(),
                                                       trg->block()->start()));
    }
  }
  return NULL;
}

Point *PatchBlock::findPoint(Location loc, Point::Type type, bool create) {
   PointMaker* maker = obj_->addrSpace()->mgr()->pointMaker();
   PatchMgrPtr mgr = obj_->addrSpace()->mgr();
   Point *ret = NULL;

   switch (type) {
      case Point::BlockEntry:
         if (!points_.entry && create) {
            points_.entry = maker->createPoint(loc, type);
         }
         return points_.entry;
         break;
      case Point::BlockExit:
         if (!points_.exit && create) {
            points_.exit = maker->createPoint(loc, type);
         }
         return points_.exit;
         break;
      case Point::BlockDuring:
         if (!points_.during && create) {
            points_.during = maker->createPoint(loc, type);
         }
         return points_.during;
         break;
      case Point::PreInsn: {
         if (!loc.addr || !loc.insn) return NULL;
         InsnPoints::iterator iter2 = points_.preInsn.find(loc.addr);
         if (iter2 == points_.preInsn.end()) {
            if (!create) return NULL;
            ret = maker->createPoint(loc, type);
            points_.preInsn[loc.addr] = ret;
            return ret;
         }
         else {
            return iter2->second;
         }
         break;
      }
      case Point::PostInsn: {
         if (!loc.addr || !loc.insn) return NULL;
         InsnPoints::iterator iter2 = points_.postInsn.find(loc.addr);
         if (iter2 == points_.postInsn.end()) {
            if (!create) return NULL;
            ret = maker->createPoint(loc, type);
            points_.preInsn[loc.addr] = ret;
            return ret;
         }
         else return iter2->second;
         break;
      }
      default:
         return NULL;
   }
   assert(0); return NULL; // unreachable, but keep compiler happy
}


void PatchBlock::remove(Point *p) {
   assert(p->block() == this);

   switch(p->type()) {
      case Point::PreInsn:
         points_.preInsn.erase(p->addr());
         break;
      case Point::PostInsn:
         points_.postInsn.erase(p->addr());
         break;
      case Point::BlockEntry:
         points_.entry = NULL;
         break;
      case Point::BlockExit:
         points_.exit = NULL;
         break;
      case Point::BlockDuring:
         points_.during = NULL;
         break;
      default:
         break;
   }
}

// destroy points for this block and then each containing function's
// context specific points for the block
void PatchBlock::destroyPoints()
{
    PatchCallback *cb = obj()->cb();
    if (points_.entry) {
        cb->destroy(points_.entry);
        delete points_.entry;
        points_.entry = NULL;
    } 
    if (points_.during) {
        cb->destroy(points_.during);
        delete points_.during;
        points_.during = NULL;
    }
    if (points_.exit) {
        cb->destroy(points_.exit);
        delete points_.exit;
        points_.exit = NULL;
    }
    if (!points_.preInsn.empty()) {
        for (InsnPoints::iterator iit = points_.preInsn.begin(); 
             iit != points_.preInsn.end(); 
             iit++) 
        {
            cb->destroy(iit->second);
            delete iit->second;
        }
        points_.preInsn.clear();
    }
    if (!points_.postInsn.empty()) {
        for (InsnPoints::iterator iit = points_.postInsn.begin(); 
             iit != points_.postInsn.end(); 
             iit++) 
        {
            cb->destroy(iit->second);
            delete iit->second;
        }
        points_.postInsn.clear();
    }

    // now destroy function's context-specific points for this block
    vector<PatchFunction *> funcs;
    getFunctions(back_inserter(funcs));
    for (vector<PatchFunction *>::iterator fit = funcs.begin();
         fit != funcs.end();
         fit++)
    {
        (*fit)->destroyBlockPoints(this);
    }
}

PatchCallback *PatchBlock::cb() const {
   return obj_->cb();
}

void PatchBlock::splitBlock(PatchBlock *succ)
{

   // Okay, get our edges right and stuff. 
   // We want to modify when possible so that we keep Points on affected edges the same. 
   // Therefore:
   // 1) Incoming edges are unchanged. 
   // 2) Outgoing edges from p1 are switched to p2 (except the fallthrough from p1 to p2)
   // 3) Fallthrough edge from p1 to p2 added if it wasn't already (depends on the status
   //    of our lazy block & edge creation when parseAPI added the edge)
   // 4) We fix up Points on the block, entry and during points stay here

   // 2)
   bool hasFTEdge = false;
   unsigned tidx= 0; 
   while (tidx < trglist_.size()) {
      PatchEdge *cur = trglist_[tidx];
      if (cur->target() == succ) {
          hasFTEdge = true;
          tidx++;
      } else {
          cur->src_ = succ;
          succ->trglist_.push_back(cur);
          int last = trglist_.size()-1;
          trglist_[tidx] = trglist_[last];
          trglist_.pop_back();
      }
   }

   // 3)
   if (!hasFTEdge) { // may have been created by ParseAPI callbacks
       ParseAPI::Block::edgelist &tmp = this->block()->targets();
       if (tmp.size() != 1) {
          cerr << "ERROR: split block has " << tmp.size() 
              << " edges, not 1 as expected!" << endl;
          assert(0);
       }
       ParseAPI::Edge *ft = *(tmp.begin());
       obj_->getEdge(ft, this, succ);
   }

   // 4)
   if (points_.exit) {
      succ->points_.exit = points_.exit;
      points_.exit = NULL;
      succ->points_.exit->changeBlock(succ);
   }

   InsnPoints::iterator pre = points_.preInsn.lower_bound(succ->start());
   for (InsnPoints::iterator tmp = pre; tmp != points_.preInsn.end(); ++tmp) {
      tmp->second->changeBlock(succ);
      succ->points_.preInsn[tmp->first] = tmp->second;
   }
   points_.preInsn.erase(pre, points_.preInsn.end());

   InsnPoints::iterator post = points_.postInsn.lower_bound(succ->start());
   for (InsnPoints::iterator tmp = post; tmp != points_.postInsn.end(); ++tmp) {
      tmp->second->changeBlock(succ);
      succ->points_.postInsn[tmp->first] = tmp->second;
   }
   points_.postInsn.erase(post, points_.postInsn.end());

}

bool PatchBlock::consistency() const {
   if (!block_) {
      cerr << "Error: block has no associated ParseAPI block, failed consistency" << endl;
      CONSIST_FAIL;
   }
   if (!srclist_.empty()) {
      if (srclist_.size() != block_->sources().size()) {
         cerr << "Error: block has inconsistent sources size" << endl;
         CONSIST_FAIL;
      }
      for (unsigned i = 0; i < srclist_.size(); ++i) {
         if (!srclist_[i]->consistency()) {
            cerr << "Error: source edge inconsistent" << endl;
            CONSIST_FAIL;
         }
      }
   }
   if (!trglist_.empty()) {
      if (trglist_.size() != block_->targets().size()) {
         cerr << "Error: block has inconsistent targets size; ParseAPI "
              << block_->targets().size() << " and PatchAPI " 
              << trglist_.size() << endl;
         CONSIST_FAIL;
      }
      for (unsigned i = 0; i < trglist_.size(); ++i) {
         if (!trglist_[i]->consistency()) {
            cerr << "Error: target edge inconsistent" << endl;
            CONSIST_FAIL;
         }
      }
   }
   if (!obj_) {
      cerr << "Error: block has no object" << endl;
      CONSIST_FAIL;
   }
   if (!points_.consistency(this, NULL)) {
      cerr << "Error: block has inconsistent points" << endl;
      CONSIST_FAIL;
   }
   return true;
}

bool BlockPoints::consistency(const PatchBlock *b, const PatchFunction *f) const {
   if (entry) {
      if (!entry->consistency()) CONSIST_FAIL;
      if (entry->block() != b) CONSIST_FAIL;
      if (entry->func() != f) CONSIST_FAIL;
      if (entry->type() != Point::BlockEntry) CONSIST_FAIL;
   }
   if (during) {
      if (!during->consistency()) CONSIST_FAIL;
      if (during->block() != b) CONSIST_FAIL;
      if (during->func() != f) CONSIST_FAIL;
      if (during->type() != Point::BlockDuring) CONSIST_FAIL;
   }
   if (exit) {
      if (!exit->consistency()) CONSIST_FAIL;
      if (exit->block() != b) CONSIST_FAIL;
      if (exit->func() != f) CONSIST_FAIL;
      if (exit->type() != Point::BlockExit) CONSIST_FAIL;
   }
   for (InsnPoints::const_iterator iter = preInsn.begin(); iter != preInsn.end(); ++iter) {
      if (!iter->second->consistency()) CONSIST_FAIL;
      if (iter->second->block() != b) CONSIST_FAIL;
      if (iter->second->func() != f) CONSIST_FAIL;
      if (iter->second->addr() != iter->first) CONSIST_FAIL;
      if (iter->second->type() != Point::PreInsn) CONSIST_FAIL;
      if (!b->getInsn(iter->first)) CONSIST_FAIL;
   }
   for (InsnPoints::const_iterator iter = postInsn.begin(); iter != postInsn.end(); ++iter) {
      if (!iter->second->consistency()) CONSIST_FAIL;
      if (iter->second->block() != b) CONSIST_FAIL;
      if (iter->second->func() != f) CONSIST_FAIL;
      if (iter->second->addr() != iter->first) CONSIST_FAIL;
      if (iter->second->type() != Point::PostInsn) CONSIST_FAIL;
      if (!b->getInsn(iter->first)) CONSIST_FAIL;
   }
   return true;
}
   
