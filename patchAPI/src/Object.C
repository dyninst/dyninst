/* Plugin */

#include "Object.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;

PatchFunction *Object::getFunction(ParseAPI::Function *f) {
   FuncMap::iterator iter = funcMap_.find(f);
   if (iter != funcMap_.end()) return iter->second;
   if (co_ != f->obj()) {
     cerr << "ERROR: function " << f->name() << " doesn't exist in this object!\n";
     exit(0);
   }
   PatchFunction *newFunc = new PatchFunction(f, shared_from_this());
   funcMap_.insert(std::make_pair(f, newFunc));
   return newFunc;
}

Object::Object(ParseAPI::CodeObject* o, Address a) : co_(o), codeBase_(a) {
  cs_ = co_->cs();
}

// Constructor for forked process
Object::Object(ObjectPtr par, ParseAPI::CodeObject* o, Address a)
  : co_(o), codeBase_(a) {
  cs_ = co_->cs();
}

void Object::destroy(ObjectPtr obj) {
  // We don't want to leak memory, so tear down the
  // entire structure
  for (FuncMap::iterator iter = obj->funcMap_.begin(); iter != obj->funcMap_.end(); ++iter) {
    delete iter->second;
  }
  obj->funcMap_.clear();
}

Object::~Object() {
   assert(funcMap_.empty());
}

void Object::setFunction(PatchFunction* f) {
  assert(f);
  f->obj_ = shared_from_this();
  funcMap_.insert(std::make_pair(f->func(), f));
}
