/* Public Interface */

#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;

// Implementation of PatchAPI edge wrapper

PatchEdge *PatchEdge::create(ParseAPI::Edge *ie, PatchBlock *src, PatchBlock *trg) {
  // Always go to the source function first. If we're an intraprocedural
  // edge we may only have the target block so use that as a backup.
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
  // TODO(wenbin): get src and trg from child
}


// In an attempt to save memory we don't create the CFG copy ahead of
// time, but instead do it on demand. This causes some interesting
// problems though.  We must create edges ahead of time if we're
// de-sharing a block. That said, we can create blocks on the fly if
// there is no ambiguity - that is, if we're in the same function.

PatchBlock *PatchEdge::source() {
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

PatchBlock *PatchEdge::target() {
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

ParseAPI::Edge *PatchEdge::edge() const {
  return edge_;
}

ParseAPI::EdgeTypeEnum PatchEdge::type() const {
  return edge_->type();
}

bool PatchEdge::sinkEdge() const {
  return edge_->sinkEdge();
}

bool PatchEdge::interproc() const {
  return edge_->interproc() ||
         (edge_->type() == ParseAPI::CALL) ||
         (edge_->type() == ParseAPI::RET);
}
