/* Public Interface */

#include "PatchCFG.h"
#include "PatchCallback.h"

using namespace Dyninst;
using namespace PatchAPI;

PatchEdge*
PatchEdge::create(ParseAPI::Edge *ie, PatchBlock *src, PatchBlock *trg) {
  PatchFunction *indexFunc = (src ? src->function() : trg->function());
  return indexFunc->object()->getEdge(ie, src, trg);
}

PatchEdge::PatchEdge(ParseAPI::Edge *internalEdge,
                     PatchBlock *source,
                     PatchBlock *target)
  : edge_(internalEdge), src_(source), trg_(target) {
}

PatchEdge::PatchEdge(const PatchEdge *parent, PatchBlock *src, PatchBlock *trg)
  : edge_(parent->edge_), src_(src), trg_(trg) {
}

PatchBlock*
PatchEdge::source() {
  if (src_) return src_;
  // Interprocedural sources _must_ be pre-created since we don't
  // have enough information to create them here.
  assert(!interproc());
  assert(trg_);
  ParseAPI::Block *isrc = edge_->src();
  if (!isrc) return NULL;
  src_ = trg_->object()->getBlock(isrc);
  return src_;
}

PatchBlock*
PatchEdge::target() {
  if (trg_) return trg_;
  assert(!interproc());
  assert(src_);
  ParseAPI::Block *itrg = edge_->trg();
  if (!itrg) return NULL;
  trg_ = src_->object()->getBlock(itrg);
  return trg_;
}

PatchEdge::~PatchEdge() {
}

ParseAPI::Edge*
PatchEdge::edge() const {
  return edge_;
}

ParseAPI::EdgeTypeEnum
PatchEdge::type() const {
  return edge_->type();
}

bool
PatchEdge::sinkEdge() const {
  return edge_->sinkEdge();
}

bool
PatchEdge::interproc() const {
  return edge_->interproc() ||
         (edge_->type() == ParseAPI::CALL) ||
         (edge_->type() == ParseAPI::RET);
}

void PatchEdge::destroy(Point *p) {
   assert(p->getEdge() == this);
   delete points_.during;
}

PatchCallback *PatchEdge::cb() const {
   return src_->object()->cb();
}
