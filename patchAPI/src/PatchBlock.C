/* Public Interface */

#include "common.h"
#include "PatchCFG.h"
#include "AddrSpace.h"
#include "PatchObject.h"
#include "PatchMgr.h"
#include "PatchCallback.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace InstructionAPI;

PatchBlock*
PatchBlock::create(ParseAPI::Block *ib, PatchFunction *f) {
  return f->object()->getBlock(ib);
}

PatchBlock::PatchBlock(ParseAPI::Block *blk, PatchObject *obj)
  : block_(blk),   obj_(obj) {
  ParseAPI::CodeObject::funclist& all = obj->co()->funcs();
  for (ParseAPI::CodeObject::funclist::iterator fit = all.begin();
       fit != all.end(); ++fit) {
    if ((*fit)->contains(blk)) {
      function_ = obj->getFunc(*fit);
      break;
    }
  }
}

PatchBlock::PatchBlock(const PatchBlock *parent, PatchObject *child)
  : block_(parent->block_), obj_(child) {
  ParseAPI::CodeObject::funclist& all = child->co()->funcs();
  for (ParseAPI::CodeObject::funclist::iterator fit = all.begin();
       fit != all.end(); ++fit) {
    if ((*fit)->contains(block_)) {
      function_ = child->getFunc(*fit);
      break;
    }
  }
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
  return function()->object()->getFunc(f);
}

PatchFunction*
PatchBlock::function() const { return function_; }

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
         return trg->function();
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

void PatchBlock::splitPoints(PatchBlock *succ) {
   // Check our points. 
   // Entry stays here
   // During stays here
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

   std::vector<PatchFunction *> funcs;
   getFunctions(std::back_inserter(funcs));
   for (unsigned i = 0; i < funcs.size(); ++i) {
      funcs[i]->splitPoints(this, succ);
   }
}

   
