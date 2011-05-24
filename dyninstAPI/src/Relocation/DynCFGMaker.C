
#include "DynObject.h"
#include "mapped_module.h"
#include "mapped_object.h"
#include "parse-cfg.h"
#include "function.h"

using Dyninst::PatchAPI::DynCFGMaker;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::ParseAPI::Function;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::ParseAPI::Block;
using Dyninst::PatchAPI::PatchBlock;

PatchFunction* DynCFGMaker::makeFunction(Function* f,
                                         PatchObject* obj) {
  Address code_base = obj->codeBase();
  mapped_object* mo = SCAST_MO(obj);
  parse_func* img_func = SCAST_PF(f);
  if (!img_func) return NULL;
  assert(img_func->getSymtabFunction());
  mapped_module* mod = mo->findModule(img_func->pdmod());
  if (!mod) {
    fprintf(stderr, "%s[%d]: ERROR: cannot find module %p\n", FILE__, __LINE__,
            img_func->pdmod());
    fprintf(stderr, "%s[%d]:  ERROR:  Cannot find module %s\n", FILE__, __LINE__,
            img_func->pdmod()->fileName().c_str());
  }
  func_instance* fi = new func_instance(img_func, code_base, mod);
  mo->addFunction(fi);
  return fi;
}

PatchFunction* DynCFGMaker::copyFunction(PatchFunction* f, PatchObject* o) {
  func_instance *parFunc = SCAST_FI(f);
  mapped_object *mo = SCAST_MO(o);
  assert(parFunc->mod());
  mapped_module *mod = mo->getOrCreateForkedModule(parFunc->mod());
  func_instance *newFunc = new func_instance(parFunc, mod);
  mo->addFunction(newFunc);
  return newFunc;
}

PatchBlock* DynCFGMaker::makeBlock(ParseAPI::Block* b, PatchObject* obj) {
  block_instance *inst = new block_instance(b, SCAST_MO(obj));
  obj->addBlock(inst);
  return inst;
}

PatchBlock* DynCFGMaker::copyBlock(PatchBlock* b, PatchObject* o) {
  block_instance *newBlock = new block_instance(SCAST_BI(b), SCAST_MO(o));
  o->addBlock(newBlock);
  return  newBlock;
}

// extern unsigned imgVarHash(const image_variable * const &func);

void DynCFGMaker::initCopiedObject(const PatchObject* parObj, PatchObject* obj) {
  /*
  mapped_object* mpar_obj = SCAST_MO(const_cast<PatchObject*>(parObj));
  mapped_object* mobj = SCAST_MO(obj);
  mobj->image_ = mpar_obj->image_;
  mobj->everyUniqueVariable = imgVarHash; 
  // Copy mapped_module
  for (unsigned k = 0; k < mpar_obj->everyModule.size(); k++) {
    mapped_module *parMod = mpar_obj->everyModule[k];
    mapped_module *mod = mapped_module::createMappedModule(mobj, parMod->pmod());
    assert(mod);
    mobj->everyModule.push_back(mod);
  }
  */
}
