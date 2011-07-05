/* Public Interface */

#include "common.h"
#include "PatchCFG.h"
#include "AddrSpace.h"
#include "PatchObject.h"
#include "PatchMgr.h"
#include "PatchCallback.h"
#include "Point.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace InstructionAPI;

PatchBlock*
PatchBlock::create(ParseAPI::Block *ib, PatchFunction *f) {
  return f->object()->getBlock(ib);
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
      trglist_.push_back(newEdge);
    }
  }
  return trglist_;
}

void PatchBlock::addSourceEdge(PatchEdge *e, bool addIfEmpty) {
   if (!addIfEmpty && srclist_.empty()) return;

   srclist_.push_back(e);

  cb()->add_edge(this, e, PatchCallback::source);
}

void PatchBlock::addTargetEdge(PatchEdge *e, bool addIfEmpty) {
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
     if ( ParseAPI::CALL == (*eit)->type() && ((*eit)->sinkEdge())) {
         return true;
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
PatchBlock::format() const {
  stringstream ret;
  ret << "BB["
      << hex << start()
      << ","
      << end() << dec
      << "]" << endl;
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
                                                       trg->start()));
    }
  }
  return NULL;
}

Point *PatchBlock::findPoint(Location loc, Point::Type type, bool create) {
   PointMakerPtr maker = obj_->addrSpace()->mgr()->pointMaker();
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


void PatchBlock::destroy(Point *p) {
   assert(p->getBlock() == this);

   switch(p->type()) {
      case Point::PreInsn:
         delete points_.preInsn[p->address()];
         break;
      case Point::PostInsn:
         delete points_.postInsn[p->address()];
         break;
      case Point::BlockEntry:
         delete points_.entry;
         break;
      case Point::BlockExit:
         delete points_.exit;
         break;
      case Point::BlockDuring:
         delete points_.during;
         break;
      default:
         break;
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
   // 2) Outgoing edges from p1 are switched to p2.
   // 3) An entirely new edge is created between p1 and p2. 
   // 4) We fix up Points on the block, entry and during points stay here

   // 2)
   for (PatchBlock::edgelist::iterator iter = this->trglist_.begin();
        iter != this->trglist_.end(); ++iter) {
      (*iter)->src_ = succ;
      succ->trglist_.push_back(*iter);
   }
   this->trglist_.clear();

   // 3)
   ParseAPI::Block::edgelist &tmp = this->block()->targets();
   if (tmp.size() != 1) {
      cerr << "Error: split block has " << tmp.size() << " edges, not 1 as expected!" << endl;
   }
   assert(tmp.size() == 1);
   ParseAPI::Edge *ft = *(tmp.begin());
   obj_->getEdge(ft, this, succ);

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
   if (!block_) return false;
   if (!srclist_.empty()) {
      if (srclist_.size() != block_->sources().size()) return false;
      for (unsigned i = 0; i < srclist_.size(); ++i) {
         if (!srclist_[i]->consistency()) return false;
      }
   }
   if (!trglist_.empty()) {
      if (trglist_.size() != block_->sources().size()) return false;
      for (unsigned i = 0; i < trglist_.size(); ++i) {
         if (!trglist_[i]->consistency()) return false;
      }
   }
   if (!obj_) return false;
   if (!points_.consistency(this, NULL)) return false;
   return true;
}

bool BlockPoints::consistency(const PatchBlock *b, const PatchFunction *f) const {
   if (entry) {
      if (!entry->consistency()) return false;
      if (entry->block() != b) return false;
      if (entry->func() != f) return false;
      if (entry->type() != Point::BlockEntry) return false;
   }
   if (during) {
      if (!during->consistency()) return false;
      if (during->block() != b) return false;
      if (during->func() != f) return false;
      if (during->type() != Point::BlockDuring) return false;
   }
   if (exit) {
      if (!exit->consistency()) return false;
      if (exit->block() != b) return false;
      if (exit->func() != f) return false;
      if (exit->type() != Point::BlockExit) return false;
   }
   for (InsnPoints::const_iterator iter = preInsn.begin(); iter != preInsn.end(); ++iter) {
      if (!iter->second->consistency()) return false;
      if (iter->second->block() != b) return false;
      if (iter->second->func() != f) return false;
      if (iter->second->addr() != iter->first) return false;
      if (iter->second->type() != Point::PreInsn) return false;
      if (!b->getInsn(iter->first)) return false;
   }
   for (InsnPoints::const_iterator iter = postInsn.begin(); iter != postInsn.end(); ++iter) {
      if (!iter->second->consistency()) return false;
      if (iter->second->block() != b) return false;
      if (iter->second->func() != f) return false;
      if (iter->second->addr() != iter->first) return false;
      if (iter->second->type() != Point::PostInsn) return false;
      if (!b->getInsn(iter->first)) return false;
   }
   return true;
}
   
