/* Plugin */

#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;


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
  funcs_.clear();
}

PatchFunction *PatchObject::getFunc(ParseAPI::Function *f) {
  if (co_ != f->obj()) {
    cerr << "ERROR: function " << f->name() << " doesn't exist in this object!\n";
    exit(0);
  }
  if (funcs_.find(f) != funcs_.end()) return funcs_[f];
  PatchFunction* newFunc = cfg_maker_->makeFunction(f, this);
  addFunc(newFunc);
  return newFunc;
}

void PatchObject::addFunc(PatchFunction* f) {
  assert(f);
  funcs_[f->function()] = f;
}

void PatchObject::removeFunc(PatchFunction* f) {
  funcs_.erase(f->function());
}
