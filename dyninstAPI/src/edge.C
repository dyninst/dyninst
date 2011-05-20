#include "block.h"
#include "function.h"
#include "mapped_object.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

edge_instance::edge_instance(ParseAPI::Edge *e, block_instance *s, block_instance *t)
  : PatchEdge(e, s, t), edge_(e), src_(s), trg_(t) {};

edge_instance::edge_instance(const edge_instance *parent, 
                             mapped_object *child)
  : PatchEdge(parent, child), 
   edge_(parent->edge_)  {
   if (parent->src_)
      src_ = child->findBlock(parent->src_->llb());
   else
      src_ = NULL;
   if (parent->trg_) 
      trg_ = child->findBlock(parent->trg_->llb());
   else
      trg_ = NULL;
}

edge_instance::~edge_instance() {
}


AddressSpace *edge_instance::proc() {
   return src()->proc();
}
