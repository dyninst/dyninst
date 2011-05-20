/* Public Interface */

#include "PatchCFG.h"
#include "PatchMgr.h"

using namespace Dyninst;
using namespace PatchAPI;

PatchFunction *PatchFunction::create(ParseAPI::Function *f, PatchObject* obj) {
  return obj->getFunction(f);
}

const PatchFunction::blocklist &PatchFunction::blocks() {
  if (!blocks_.empty()) return blocks_;
  // Otherwise we need to create them
  for (ParseAPI::Function::blocklist::iterator iter = func_->blocks().begin();
       iter != func_->blocks().end(); ++iter) {
    blocks_.push_back(getBlock(*iter));
  }
  return blocks_;
}

const PatchFunction::edgelist &PatchFunction::callEdges() {
  if (!callEdges_.empty()) return callEdgeList_;
  // Build it
  for (ParseAPI::Function::edgelist::iterator iter = func_->callEdges().begin();
       iter != func_->callEdges().end(); ++iter) {
    PatchBlock *callBlock = getBlock((*iter)->src());
    for (PatchBlock::edgelist::iterator e_iter = callBlock->targets().begin();
         e_iter != callBlock->targets().end(); ++e_iter) {
      if ((*e_iter)->type() == ParseAPI::CALL) {
        callEdges_.push_back(*e_iter);
      }
    }
  }
  return callEdgeList_;
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

PatchEdge *PatchFunction::getEdge(ParseAPI::Edge *iedge,
                                  PatchBlock *src,
                                  PatchBlock *trg) {
  EdgeMap::iterator iter = edgeMap_.find(iedge);
  if (iter != edgeMap_.end()) {
    // Consistency check
    if (src) assert(iter->second->src() == src);
    if (trg) assert(iter->second->trg() == trg);
    return iter->second;
  }

  PatchEdge *newEdge = new PatchEdge(iedge, src, trg);
  edgeMap_.insert(std::make_pair(iedge, newEdge));
  return newEdge;
}

void PatchFunction::removeEdge(PatchEdge *e) {
  edgeMap_.erase(e->edge());
}

void PatchFunction::destroy(PatchFunction *f) {
  for (blocklist::iterator iter = f->blocks_.begin(); iter != f->blocks_.end();
       ++iter) {
    PatchBlock::destroy(*iter);
  }
  f->blocks_.clear();
  f->callEdges_.clear();
  f->returnBlocks_.clear();
  f->blockMap_.clear();
  f->edgeMap_.clear();
  delete f;
}

PatchFunction::~PatchFunction() {
  /*
    assert(blocks_.empty());
    assert(callEdges_.empty());
    assert(returnBlocks_.empty());
    assert(blockMap_.empty());
    assert(edgeMap_.empty());
  */
}

PatchFunction* PatchFunction::convert(int_function* /*ifun*/) {
  return NULL;
}

bool PatchFunction::entries(PointSet& pts) {
  obj_->addrSpace()->mgr_->findPoints(this, Point::FuncEntry, inserter(pts, pts.begin()));
  assert(pts.size() > 0);
  return true;
}

bool PatchFunction::exits(PointSet& pts) {
  obj_->addrSpace()->mgr_->findPoints(this, Point::FuncExit, inserter(pts, pts.begin()));
  assert(pts.size() > 0);
  return true;
}
