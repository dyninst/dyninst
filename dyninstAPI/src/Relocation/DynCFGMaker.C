#include "DynObject.h"
#include "mapped_module.h"
#include "mapped_object.h"
#include "parse-cfg.h"
#include "function.h"

using Dyninst::PatchAPI::DynCFGMaker;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::ParseAPI::Function;
using Dyninst::PatchAPI::PatchFunction;

PatchFunction* DynCFGMaker::makeFunction(Function* f,
                                         PatchObject* obj) {
  Address code_base = obj->codeBase();
  mapped_object* mo = static_cast<mapped_object*>(obj);
  parse_func* img_func = static_cast<parse_func*>(f);
  if (!img_func) return NULL;
  assert(img_func->getSymtabFunction());
  mapped_module* mod = mo->findModule(img_func->pdmod());
  if (!mod) {
    fprintf(stderr, "%s[%d]: ERROR: cannot find module %p\n", FILE__, __LINE__, img_func->pdmod());
    fprintf(stderr, "%s[%d]:  ERROR:  Cannot find module %s\n", FILE__, __LINE__, img_func->pdmod()->fileName().c_str());
  }
  func_instance* fi = new func_instance(img_func, code_base, mod);
  mo->addFunction(fi);
  return fi;
}

PatchFunction* DynCFGMaker::copyFunction(PatchFunction* f, PatchObject* o) {
  func_instance *parFunc = static_cast<func_instance*>(f);
  mapped_object *mo = static_cast<mapped_object*>(o); 
  assert(parFunc->mod());
  mapped_module *mod = mo->getOrCreateForkedModule(parFunc->mod());
  func_instance *newFunc = new func_instance(parFunc, mod);
  mo->addFunction(newFunc);
  return newFunc;
}

