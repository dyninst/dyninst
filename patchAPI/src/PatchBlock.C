/* Public Interface */

#include "common.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace InstructionAPI;

PatchBlock *PatchBlock::create(ParseAPI::Block *ib, PatchFunction *f) {
  return f->getBlock(ib);
}

void PatchBlock::getInsns(InsnInstances &insns) {
  // Pass through to ParseAPI. They don't have a native interface, so add one.
  Offset off = block_->start();
  const unsigned char *ptr =
    (const unsigned char *)block_->region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionDecoder d(ptr, size(), block_->obj()->cs()->getArch());
  while (off < block_->end()) {
    insns.push_back(std::make_pair(off, d.decode()));
    off += insns.back().second->size();
  }
}

void PatchBlock::createInterproceduralEdges(ParseAPI::Edge *iedge,
                                            Direction dir,
                                            std::vector<PatchEdge *> &edges) {
  // Let pT be the target block in the parseAPI
  // Let {f_1, ..., f_n} be the functions T is in
  // We create blocks t_i for each function f_i
  ParseAPI::Block *iblk = (dir == backwards) ? iedge->src() : iedge->trg();
  if (!iblk) {
    assert(dir == forwards); // Can't have sink in-edges

    edges.push_back(PatchEdge::create(iedge, this, NULL));
    return;
  }
  std::vector<ParseAPI::Function *> ifuncs;
  iblk->getFuncs(ifuncs);
  for (unsigned i = 0; i < ifuncs.size(); ++i) {
    PatchFunction *pfunc = object()->getFunction(ifuncs[i]);
    assert(pfunc);
    PatchBlock *pblock = pfunc->getBlock(iblk);
    assert(pblock);
    PatchEdge *newEdge = NULL;
    if (dir == forwards)
      newEdge = PatchEdge::create(iedge, this, pblock);
    else
      newEdge = PatchEdge::create(iedge, pblock, this);

    edges.push_back(newEdge);
  }
  return;
}


const PatchBlock::edgelist &PatchBlock::sources() {
  if (srcs_.empty()) {
    for (ParseAPI::Block::edgelist::iterator iter = block_->sources().begin();
         iter != block_->sources().end(); ++iter) {
      // We need to copy interprocedural edges to ensure that we de-overlap
      // code in functions. We do this here.
      // XXX
      //if ((*iter)->interproc()) {
      if ((*iter)->type() == ParseAPI::CALL) {
        createInterproceduralEdges(*iter, backwards, srcs_);
      }
      else {
        // Can lazily create the source block since it's in
        // our function.
        PatchEdge *newEdge = PatchEdge::create(*iter, NULL, this);
        srcs_.push_back(newEdge);
      }
    }
  }
  return srclist_;
}

const PatchBlock::edgelist &PatchBlock::targets() {
  if (trgs_.empty()) {
    for (ParseAPI::Block::edgelist::iterator iter = block_->targets().begin();
         iter != block_->targets().end(); ++iter) {
      // We need to copy interprocedural edges to ensure that we de-overlap
      // code in functions. We do this here.
      // XXX: this doesn't work!
      //         if ((*iter)->interproc()) {
      if ((*iter)->type() == ParseAPI::CALL) {
        createInterproceduralEdges(*iter, forwards, trgs_);
      }
      else {
        // Can lazily create the source block since it's in
        // our function.
        PatchEdge *newEdge = PatchEdge::create(*iter, this, NULL);
        trgs_.push_back(newEdge);
      }
    }
  }
  return trglist_;
}

PatchObjectPtr PatchBlock::object() const {
  assert(function_);
  return function_->object();
}

PatchBlock::PatchBlock(ParseAPI::Block *block,
                       PatchFunction *func)
   : block_(block),
     function_(func),
     srclist_(srcs_),
     trglist_(trgs_) {};

void PatchBlock::destroy(PatchBlock *b) {
  // As a note, deleting edges that source and target this
  // block is an exercise in delicacy. Make sure you know
  // what you're doing. For this, we ensure that we always
  // remove source edges first, and that we can't accidentally
  // invalidate an iterator.
  for (std::vector<PatchEdge *>::iterator iter = b->srcs_.begin();
       iter != b->srcs_.end(); ++iter) {
    if ((*iter)->src_) {
      (*iter)->src_->removeTargetEdge(*iter);
    }
    PatchEdge::destroy(*iter);
  }
  b->srcs_.clear();
  for (std::vector<PatchEdge *>::iterator iter = b->trgs_.begin();
       iter != b->trgs_.end(); ++iter) {
    if ((*iter)->trg_) {
      (*iter)->trg_->removeSourceEdge(*iter);
    }
    PatchEdge::destroy(*iter);
  }
  b->trgs_.clear();
  delete b;
}

void PatchBlock::removeSourceEdge(PatchEdge *e) {
  // This is called as part of teardown
  // of another Block to remove its edges from our
  // vectors.
  for (std::vector<PatchEdge *>::iterator iter = srcs_.begin();
       iter != srcs_.end(); ++iter) {
    if ((*iter) == e) {
      srcs_.erase(iter);
      return;
    }
  }
}

void PatchBlock::removeTargetEdge(PatchEdge *e) {
  // This is called as part of teardown
  // of another Block to remove its edges from our
  // vectors.
  for (std::vector<PatchEdge *>::iterator iter = trgs_.begin();
       iter != trgs_.end(); ++iter) {
    if ((*iter) == e) {
      trgs_.erase(iter);
      return;
    }
  }
}


bool PatchBlock::isShared() {
  std::vector<ParseAPI::Function*> funcs;
  block()->getFuncs(funcs);
  if (funcs.size() > 1) return true;
  return false;
}
PatchBlock::~PatchBlock() {
  // We assume top-down teardown of data
  //   assert(srcs_.empty());
  //   assert(trgs_.empty());
}

PatchBlock* PatchBlock::convert(int_block* /*ib*/) {
  return NULL;
}
