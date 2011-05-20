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

void PatchObject::destroy(PatchObject* obj) {
  // We don't want to leak memory, so tear down the
  // entire structure
  for (FuncMap::iterator iter = obj->funcMap_.begin(); iter != obj->funcMap_.end(); ++iter) {
    PatchFunction::destroy(iter->second);
    //delete iter->second;
  }
  obj->funcMap_.clear();
}

PatchObject::~PatchObject() {
  assert(funcMap_.empty());
}

PatchFunction *PatchObject::getFunc(ParseAPI::Function *f) {
   FuncMap::iterator iter = funcMap_.find(f);
   if (iter != funcMap_.end()) return iter->second;
   if (co_ != f->obj()) {
     cerr << "ERROR: function " << f->name() << " doesn't exist in this object!\n";
     exit(0);
   }
   PatchFunction *newFunc = new PatchFunction(f, this);
   funcMap_.insert(std::make_pair(f, newFunc));
   return newFunc;
}

void PatchObject::addFunc(PatchFunction* f) {
  assert(f);
  f->obj_ = this;
  funcMap_.insert(std::make_pair(f->func(), f));
}

void PatchObject::removeFunc(PatchFunction* f) {
  funcMap_.erase(f->func());
}
