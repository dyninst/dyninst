/* Plugin */

#include "PatchObject.h"
#include "PatchCFG.h"
#include "AddrSpace.h"
#include "PatchMgr.h"
#include "PatchCallback.h"
#include "ParseCallback.h"

using namespace Dyninst;
using namespace PatchAPI;


PatchObject*
PatchObject::create(ParseAPI::CodeObject* co, Address base, CFGMakerPtr cm, PatchCallback *cb) {
   PatchObject* obj = new PatchObject(co, base, cm, cb);
   return obj;
}

PatchObject*
PatchObject::clone(PatchObject* par_obj, Address base, PatchCallback *cb) {
   PatchObject* obj = new PatchObject(par_obj, base, cb);
   obj->copyCFG(par_obj);
   return obj;
}

PatchObject::PatchObject(ParseAPI::CodeObject* o, Address a, CFGMakerPtr cm, PatchCallback *cb)
  // : co_(o), cs_(o->cs()), codeBase_(a), cfg_maker_(cm) {
  : co_(o), codeBase_(a), cfg_maker_(cm) {
   if (!cb) { 
      cb_ = new PatchCallback();
   }
   else {
      cb_ = cb;
   }
   // Set up a new callback
   PatchParseCallback *pcb = new PatchParseCallback(this);
   co_->registerCallback(pcb);

}

PatchObject::PatchObject(const PatchObject* parObj, Address a, PatchCallback *cb)
  // : co_(parObj->co()), cs_(parObj->cs()), codeBase_(a), cfg_maker_(parObj->cfg_maker_) {
  : co_(parObj->co()), codeBase_(a), cfg_maker_(parObj->cfg_maker_) {
   if (!cb) {
      cb_ = new PatchCallback();
   }
   else  {
      cb_ = cb;
   }

   // Set up a new callback
   PatchParseCallback *pcb = new PatchParseCallback(this);
   co_->registerCallback(pcb);
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
  delete cb_;
}

PatchFunction*
PatchObject::getFunc(ParseAPI::Function *f, bool create) {
  if (co_ != f->obj()) {
    cerr << "ERROR: function " << f->name() << " doesn't exist in this object!\n";
    assert(0);
  }

  FuncMap::iterator iter = funcs_.find(f);
  if (iter != funcs_.end()) return iter->second;
  else if (!create) return NULL;

  PatchFunction *ret = cfg_maker_->makeFunction(f, this);
  addFunc(ret);
  return ret;
}

void
PatchObject::addFunc(PatchFunction* f) {
  assert(f);
  funcs_[f->function()] = f;
  cb()->create(f);
}

void
PatchObject::removeFunc(PatchFunction* f) {
   removeFunc(f->function());
}

void
PatchObject::removeFunc(ParseAPI::Function* f) {
   FuncMap::iterator iter = funcs_.find(f);
   if (iter == funcs_.end()) return;
   cb()->destroy(iter->second);
   funcs_.erase(iter);
}

PatchBlock*
PatchObject::getBlock(ParseAPI::Block* b, bool create) {
  if (co_ != b->obj()) {
    cerr << "ERROR: block starting at 0x" << b->start()
         << " doesn't exist in this object!\n";
    exit(0);
  }
  BlockMap::iterator iter = blocks_.find(b);
  if (iter != blocks_.end()) return iter->second;
  else if (!create) return NULL;

  PatchBlock *ret = cfg_maker_->makeBlock(b, this);
  addBlock(ret);
  return ret;
}

void
PatchObject::addBlock(PatchBlock* b) {
  assert(b);
  blocks_[b->block()] = b;
  cb()->create(b);
}

void
PatchObject::removeBlock(PatchBlock* b) {
   removeBlock(b->block());
}

void
PatchObject::removeBlock(ParseAPI::Block* b) {
   BlockMap::iterator iter = blocks_.find(b);
   if (iter == blocks_.end()) return;
   cb()->destroy(iter->second);
   blocks_.erase(iter);
}

PatchEdge*
PatchObject::getEdge(ParseAPI::Edge* e, PatchBlock* src, PatchBlock* trg, bool create) {
   EdgeMap::iterator iter = edges_.find(e);
   if (iter != edges_.end()) return iter->second;
   else if (!create) return NULL;

   PatchEdge *ret = cfg_maker_->makeEdge(e, src, trg, this);
   addEdge(ret);
   return ret;
}

void
PatchObject::addEdge(PatchEdge* e) {
  assert(e);
  edges_[e->edge()] = e;
  cb()->create(e);
}

void
PatchObject::removeEdge(PatchEdge* e) {
   removeEdge(e->edge());
}

void
PatchObject::removeEdge(ParseAPI::Edge *e) {
   EdgeMap::iterator iter = edges_.find(e);
   if (iter == edges_.end()) return;
   cb()->destroy(iter->second);
   edges_.erase(iter);
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


bool PatchObject::splitBlock(PatchBlock *p1, ParseAPI::Block *second) {
   PatchBlock *p2 = getBlock(second, false);
   if (p2) 
       return true; // ???
   p2 = getBlock(second);

   return true;
}

std::string PatchObject::format() const {
   ParseAPI::CodeSource *cs = co()->cs();
   ParseAPI::SymtabCodeSource *symtab = dynamic_cast<ParseAPI::SymtabCodeSource *>(cs);
   stringstream ret;

   if (symtab) {
      ret << symtab->getSymtabObject()->name();
   }

   ret << hex << "(" << this << ")" << dec;
   return ret.str();

}

bool PatchObject::consistency(const AddrSpace *as) const {
   if (!co_) return false;
   if (as != addr_space_.get()) return false;

   for (FuncMap::const_iterator iter = funcs_.begin(); iter != funcs_.end(); ++iter) {
      if (!iter->second->consistency()) {
         cerr << "Error: " << iter->second->name() << " failed consistency!" << endl;
         return false;
      }
   }
   for (BlockMap::const_iterator iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
      if (!iter->second->consistency()) {
         cerr << "Error: block @ " << hex << iter->second->start() << " failed consistency" << endl;
         return false;
      }
   }
   for (EdgeMap::const_iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
      if (!iter->second->consistency()) return false;
   }
   if (!cfg_maker_) return false;
   if (!cb_) return false;
   return true;
}
