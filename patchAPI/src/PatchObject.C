/* Plugin */

#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;


PatchObject*
PatchObject::create(ParseAPI::CodeObject* co, Address base, CFGMakerPtr cm) {
  PatchObject* obj = new PatchObject(co, base, cm);
  return obj;
}

PatchObject*
PatchObject::clone(PatchObject* par_obj, Address base) {
  PatchObject* obj = new PatchObject(par_obj, base);
  obj->copyCFG(par_obj);
  return obj;
}

PatchObject::PatchObject(ParseAPI::CodeObject* o, Address a, CFGMakerPtr cm)
  : co_(o), cs_(o->cs()), codeBase_(a), cfg_maker_(cm) {
}

PatchObject::PatchObject(const PatchObject* parObj, Address a)
  : co_(parObj->co()), cs_(parObj->cs()), codeBase_(a), cfg_maker_(parObj->cfg_maker_) {

}

PatchObject::~PatchObject() {
  for (FuncMap::iterator iter = funcs_.begin(); iter != funcs_.end(); ++iter) {
    delete iter->second;
  }
  for (BlockMap::iterator iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
    delete iter->second;
  }
  for (EdgeMap::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
    delete iter->second;
  }
}

PatchFunction*
PatchObject::getFunc(ParseAPI::Function *f) {
  if (co_ != f->obj()) {
    cerr << "ERROR: function " << f->name() << " doesn't exist in this object!\n";
    exit(0);
  }
  if (funcs_.find(f) != funcs_.end()) return funcs_[f];
  PatchFunction* newFunc = cfg_maker_->makeFunction(f, this);
  addFunc(newFunc);
  return newFunc;
}

void
PatchObject::addFunc(PatchFunction* f) {
  assert(f);
  funcs_[f->function()] = f;
}

void
PatchObject::removeFunc(PatchFunction* f) {
  funcs_.erase(f->function());
}

PatchBlock*
PatchObject::getBlock(ParseAPI::Block* b) {
  if (co_ != b->obj()) {
    cerr << "ERROR: block starting at 0x" << b->start()
         << " doesn't exist in this object!\n";
    exit(0);
  }
  if (blocks_.find(b) != blocks_.end()) return blocks_[b];
  PatchBlock *new_block = cfg_maker_->makeBlock(b, this);
  addBlock(new_block);
  return new_block;
}

void
PatchObject::addBlock(PatchBlock* b) {
  assert(b);
  blocks_[b->block()] = b;
}

void
PatchObject::removeBlock(PatchBlock* b) {
  blocks_.erase(b->block());
}

PatchEdge*
PatchObject::getEdge(ParseAPI::Edge* e, PatchBlock* src, PatchBlock* trg) {
  if (edges_.find(e) != edges_.end()) return edges_[e];
  PatchEdge* new_edge = cfg_maker_->makeEdge(e, src, trg, this);
  addEdge(new_edge);
  return new_edge;
}

void
PatchObject::addEdge(PatchEdge* e) {
  assert(e);
  edges_[e->edge()] = e;
}

void
PatchObject::removeEdge(PatchEdge* e) {
  edges_.erase(e->edge());
}

void
PatchObject::copyCFG(PatchObject* parObj) {
  for (EdgeMap::const_iterator iter = parObj->edges_.begin();
       iter != parObj->edges_.end(); ++iter) {
    cfg_maker_->copyEdge(iter->second, this);
  }
  // Duplicate all copied blocks
  for (BlockMap::const_iterator iter = parObj->blocks_.begin();
       iter != parObj->blocks_.end(); ++iter) {
    cfg_maker_->copyBlock(iter->second, this);
  }
  // Aaand now functions
  for (FuncMap::const_iterator iter = parObj->funcs_.begin();
       iter != parObj->funcs_.end(); ++iter) {
    cfg_maker_->copyFunction(iter->second, this);
  }
}
