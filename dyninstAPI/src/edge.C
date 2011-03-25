#include "block.h"
#include "function.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

edge_instance::edge_instance(ParseAPI::Edge *e, block_instance *s, block_instance *t)
   : edge_(e), src_(s), trg_(t) {};

edge_instance::~edge_instance() {
}


AddressSpace *edge_instance::proc() {
   return src()->proc();
}
