#include "block.h"
#include "function.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

edge_instance *edge_instance::create(ParseAPI::Edge *e, block_instance *s, block_instance *t) {
   // Always go to the source function first. If we're an intraprocedural
   // edge we may only have the target block so use that as a backup.
   func_instance *indexFunc = (s ? s->func() : t->func());
   return indexFunc->getEdge(e, s, t);
}

// Upcalls suck, but someone has to let the function
// know to remove us from the edge map.
void edge_instance::destroy(edge_instance *e) {
   func_instance *indexFunc = (e->src_ ? e->src_->func() : e->trg_->func());
   indexFunc->removeEdge(e);
   
   delete e;
}

edge_instance::edge_instance(ParseAPI::Edge *e, block_instance *s, block_instance *t)
   : edge_(e), src_(s), trg_(t), bpEdge_(NULL) {};

// In an attempt to save memory we don't create the CFG copy ahead of
// time, but instead do it on demand. This causes some interesting
// problems though.  We must create edges ahead of time if we're
// de-sharing a block. That said, we can create blocks on the fly if
// there is no ambiguity - that is, if we're in the same function.

block_instance *edge_instance::src() {
   if (src_) return src_;

   // Interprocedural sources _must_ be pre-created since we don't
   // have enough information to create them here.
   assert(!interproc());   
   assert(trg_);

   ParseAPI::Block *isrc = edge_->src();
   if (!isrc) return NULL;

   src_ = trg_->func()->findBlock(isrc);
   return src_;
}

block_instance *edge_instance::trg() {
   if (trg_) return trg_;
   
   assert(!interproc());   
   assert(src_);

   ParseAPI::Block *itrg = edge_->trg();
   if (!itrg) return NULL;

   trg_ = src_->func()->findBlock(itrg);
   return trg_;
}

instPoint *edge_instance::point() {
   instPoint *point = findPoint(instPoint::Edge);
   if (point) return point;
   
   instPoint *newPoint = instPoint::edge(this);
   func()->edgePoints_[this] = newPoint;
   return newPoint;
}

instPoint *edge_instance::findPoint(instPoint::Type type) {
   if (type != instPoint::Edge) return NULL;


   func_instance::EdgePointMap::iterator iter = func()->edgePoints_.find(this);
   if (iter != func()->edgePoints_.end()) return iter->second;
   return NULL;
}

edge_instance::~edge_instance() {
   assert(src_ == NULL);
   assert(trg_ == NULL);
}

func_instance *edge_instance::func() {
   if (src_) return src_->func();
   return trg_->func();
}

AddressSpace *edge_instance::proc() {
   return func()->proc();
}
