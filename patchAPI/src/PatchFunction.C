/* Public Interface */

#include "PatchCFG.h"
#include "PatchMgr.h"

using namespace Dyninst;
using namespace PatchAPI;

PatchFunction *PatchFunction::create(ParseAPI::Function *f, PatchObject* obj) {
  return obj->getFunc(f);
}

PatchFunction::PatchFunction(ParseAPI::Function *f,
     PatchObject* o) : func_(f), obj_(o), addr_(obj_->codeBase() + func_->addr()) {}

PatchFunction::PatchFunction(const PatchFunction *parFunc, PatchObject* child)
  : func_(parFunc->func_), obj_(child), addr_(obj_->codeBase() + func_->addr()) {}

const PatchFunction::blocklist &PatchFunction::blocks() {
  if (!blocks_.empty()) return blocks_;
  // Otherwise we need to create them
  for (ParseAPI::Function::blocklist::iterator iter = func_->blocks().begin();
       iter != func_->blocks().end(); ++iter) {
    blocks_.push_back(getBlock(*iter));
  }
  return blocks_;
}

const PatchFunction::blocklist &PatchFunction::returnBlocks() {
  if (!returnBlocks_.empty()) return returnBlocks_;

  for (ParseAPI::Function::blocklist::iterator iter = func_->returnBlocks().begin();
       iter != func_->returnBlocks().end(); ++iter) {
    PatchBlock* pblk = getBlock(*iter);
    returnBlocks_.push_back(pblk);
  }
  return returnBlocks_;
}

PatchBlock *PatchFunction::getBlock(ParseAPI::Block *iblock) {
  BlockMap::iterator iter = blockMap_.find(iblock);
  if (iter != blockMap_.end()) return iter->second;

  PatchBlock *newBlock = new PatchBlock(iblock, this);
  blockMap_.insert(std::make_pair(iblock, newBlock));
  return newBlock;
}
void PatchFunction::addBlock(PatchBlock* b) {
  assert(b);
  blockMap_.insert(std::make_pair(b->block_, b));
}

PatchEdge *PatchFunction::getEdge(ParseAPI::Edge *iedge,
                                  PatchBlock *src,
                                  PatchBlock *trg) {
  EdgeMap::iterator iter = edgeMap_.find(iedge);
  if (iter != edgeMap_.end()) {
    // Consistency check
    if (src) assert(iter->second->source() == src);
    if (trg) assert(iter->second->target() == trg);
    return iter->second;
  }

  PatchEdge *newEdge = new PatchEdge(iedge, src, trg);
  edgeMap_.insert(std::make_pair(iedge, newEdge));
  return newEdge;
}

void PatchFunction::removeEdge(PatchEdge *e) {
  edgeMap_.erase(e->edge());
}


PatchFunction::~PatchFunction() {
  for (blocklist::iterator iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
    delete *iter;
  }
  blocks_.clear();
  callEdges_.clear();
  returnBlocks_.clear();
  blockMap_.clear();
  edgeMap_.clear();
}
