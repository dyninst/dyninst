#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;

/* Default Implementation for CFGMaker */

PatchFunction*
CFGMaker::makeFunction(ParseAPI::Function* f, PatchObject* obj) {
  PatchFunction* ret = new PatchFunction(f, obj);
  if (!ret) {
    cerr << "ERROR: Cannot create PatchFunction for " << f->name() << "\n";
    assert(0);
  }
  return ret;
}

PatchFunction*
CFGMaker::copyFunction(PatchFunction* f, PatchObject* obj) {
  return (new PatchFunction(f, obj));
}

PatchBlock*
CFGMaker::makeBlock(ParseAPI::Block* b, PatchObject* obj) {
  PatchBlock* ret = new PatchBlock(b, obj);
  if (!ret) {
    cerr << "ERROR: Cannot create PatchBlock for 0x" << std::hex
         << b->start() << "\n" << std::dec;
    assert(0);
  }
  return ret;
}

PatchBlock*
CFGMaker::copyBlock(PatchBlock* b, PatchObject* obj) {
  return (new PatchBlock(b, obj));
}

PatchEdge*
CFGMaker::makeEdge(ParseAPI::Edge* e, PatchBlock* s, PatchBlock* t, PatchObject* o) {
  return (new PatchEdge(e,
                        s ? s : o->getBlock(e->src()),
                        t ? t : o->getBlock(e->trg())));
}

PatchEdge*
CFGMaker::copyEdge(PatchEdge* e, PatchObject* o) {
  return (new PatchEdge(e,
                        o->getBlock(e->source()->block()),
                        o->getBlock(e->target()->block())));
}
