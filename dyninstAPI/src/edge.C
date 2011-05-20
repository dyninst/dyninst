#include "block.h"
#include "function.h"
#include "mapped_object.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

edge_instance::edge_instance(ParseAPI::Edge *e, block_instance *s, block_instance *t)
  : PatchEdge(e, s, t) {
}

// For forked process
edge_instance::edge_instance(const edge_instance *parent,
                             mapped_object *child)
  : PatchEdge(parent,
              parent->src_?child->findBlock(parent->src()->llb()):NULL,
              parent->trg_?child->findBlock(parent->trg()->llb()):NULL) {
}

edge_instance::~edge_instance() {
}


AddressSpace *edge_instance::proc() {
   return src()->proc();
}

block_instance *edge_instance::src() const {
  return DYN_CAST_BI(src_);
}

block_instance *edge_instance::trg() const {
  return DYN_CAST_BI(trg_);
}
