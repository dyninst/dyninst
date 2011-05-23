/* Plugin */

#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;


PatchObject::PatchObject(ParseAPI::CodeObject* o, Address a)
  : co_(o), cs_(o->cs()), codeBase_(a) {
}

PatchObject::PatchObject(const PatchObject* parObj, Address a)
  : co_(parObj->co()), cs_(parObj->cs()), codeBase_(a) {
}

PatchObject::~PatchObject() {
  for (FuncMap::iterator iter = funcs_.begin(); iter != funcs_.end(); ++iter) {
    delete iter->second;
  }
  funcs_.clear();
}

PatchFunction *PatchObject::getFunc(ParseAPI::Function *f) {
   FuncMap::iterator iter = funcs_.find(f);
   if (iter != funcs_.end()) return iter->second;
   if (co_ != f->obj()) {
     cerr << "ERROR: function " << f->name() << " doesn't exist in this object!\n";
     exit(0);
   }

   //cerr << "creating " << f->name() << "\n";
   CFGMaker cfg_maker_;
   PatchFunction* newFunc = cfg_maker_.makeFunction(f, this);
   addFunc(newFunc);
   return newFunc;
}

void PatchObject::addFunc(PatchFunction* f) {
  assert(f);
  f->obj_ = this;
  funcs_.insert(std::make_pair(f->function(), f));
}

void PatchObject::removeFunc(PatchFunction* f) {
  funcs_.erase(f->function());
}
