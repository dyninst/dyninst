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
PatchObject::create(ParseAPI::CodeObject* co, Address base, CFGMaker* cm, PatchCallback *cb) {
  patchapi_debug("Create PatchObject at %lx", base);
   PatchObject* obj = new PatchObject(co, base, cm, cb);
   return obj;
}

PatchObject*
PatchObject::clone(PatchObject* par_obj, Address base, CFGMaker* cm, PatchCallback *cb) {
  patchapi_debug("Clone PatchObject at %lx", base);
  PatchObject* obj = new PatchObject(par_obj, base, cm, cb);
  obj->copyCFG(par_obj);
  return obj;
}

PatchObject::PatchObject(ParseAPI::CodeObject* o, Address a, CFGMaker* cm, PatchCallback *cb)
   : co_(o), codeBase_(a),
     addr_space_(NULL),
     cfg_maker_(NULL)
{
  if (!cm) {
     patchapi_debug("Use default CFGMaker");
     cfg_maker_ = new CFGMaker;
  } else {
     patchapi_debug("Use plugin CFGMaker");
     cfg_maker_ = cm;
  }
   if (!cb) { 
     patchapi_debug("Use default PatchCallback");
      cb_ = new PatchCallback();
   }
   else {
     patchapi_debug("Use plugin PatchCallback");
     cb_ = cb;
   }
   // Set up a new callback
   pcb_ = new PatchParseCallback(this);
   co_->registerCallback(pcb_);
}

PatchObject::PatchObject(const PatchObject* parObj, Address a, CFGMaker* cm, PatchCallback *cb)
  : co_(parObj->co()), codeBase_(a) {
  if (!cm) {
    patchapi_debug("Use default PatchObject");
    cfg_maker_ = new CFGMaker;
  } else {
    patchapi_debug("Use plugin PatchObject");
    cfg_maker_ = cm;
  }
   if (!cb) {
    patchapi_debug("Use default PatchCallback");
    cb_ = new PatchCallback();
   }
   else  {
    patchapi_debug("Use plugin PatchCallback");
    cb_ = cb;
   }

   // Set up a new callback
   pcb_ = new PatchParseCallback(this);
   co_->registerCallback(pcb_);
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
  co_->unregisterCallback(pcb_);
  delete cfg_maker_;
  delete cb_;
}

PatchFunction*
PatchObject::getFunc(ParseAPI::Function *f, bool create) {
  if (!f) return NULL;

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
   funcs_.erase(iter);
}

PatchBlock*
PatchObject::getBlock(ParseAPI::Block* b, bool create) {
  if (co_ != b->obj()) {
    cerr << "ERROR: block starting at 0x" << b->start()
         << " doesn't exist in this object!\n";
    cerr << "This: " << hex << this << " and our code object: " << co_ << " and block is " << b->obj() << dec << endl;
    assert(0);
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
   blocks_.erase(iter);
}

PatchEdge*
PatchObject::getEdge(ParseAPI::Edge* e, PatchBlock* src, PatchBlock* trg, bool create) {
   EdgeMap::iterator iter = edges_.find(e);
   if (iter != edges_.end()) return iter->second;
   else if (!create) return NULL;

   // We can only create if we have src or trg
   if (!src && !trg) return NULL;

   PatchEdge *ret = cfg_maker_->makeEdge(e, src, trg, this);
   addEdge(ret);
   if (ret->trg()->obj() != this) {
      ret->trg()->obj()->addEdge(ret);
   }
   cb()->create(ret);
   return ret;
}

void
PatchObject::addEdge(PatchEdge* e) {
  assert(e);
  edges_[e->edge()] = e;
}

void
PatchObject::removeEdge(PatchEdge* e) {
   removeEdge(e->edge());
}

void
PatchObject::removeEdge(ParseAPI::Edge *e) {
   EdgeMap::iterator iter = edges_.find(e);
   if (iter == edges_.end()) return;
   edges_.erase(iter);
}

void
PatchObject::copyCFG(PatchObject* parObj) {
  for (EdgeMap::const_iterator iter = parObj->edges_.begin();
       iter != parObj->edges_.end(); ++iter) {
     edges_[iter->first] = cfg_maker_->copyEdge(iter->second, this);
  }
  // Duplicate all copied blocks
  for (BlockMap::const_iterator iter = parObj->blocks_.begin();
       iter != parObj->blocks_.end(); ++iter) {
     blocks_[iter->first] = cfg_maker_->copyBlock(iter->second, this);
  }
  // Aaand now functions
  for (FuncMap::const_iterator iter = parObj->funcs_.begin();
       iter != parObj->funcs_.end(); ++iter) {
     funcs_[iter->first] = cfg_maker_->copyFunction(iter->second, this);
  }
}


bool PatchObject::splitBlock(PatchBlock * /*orig*/, ParseAPI::Block *second) {
   PatchBlock *p2 = getBlock(second, false);
   if (p2) return true;
   p2 = getBlock(second);

   return true;
}

std::string PatchObject::format() const {
   ParseAPI::CodeSource *cs = co()->cs();
   ParseAPI::SymtabCodeSource *symtab = dynamic_cast<ParseAPI::SymtabCodeSource *>(cs);
   stringstream ret;

   if (symtab) {
      //ret << symtab->getSymtabObject()->name();
   }

   ret << hex << "(" << this << ")" << dec;
   return ret.str();

}

bool PatchObject::consistency(const AddrSpace *as) const {
   if (!co_) return false;
   if (as != addr_space_) return false;

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

PatchMgrPtr PatchObject::mgr() const { 
   return addr_space_->mgr();
}

void PatchObject::setAddrSpace(AddrSpace *as) {
   addr_space_ = as;
}
