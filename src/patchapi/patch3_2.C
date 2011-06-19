/*
 * #Name: patch3_2
 * #Desc: replace function
 * #Dep:
  //  Mutatee function replacement scheme:
  //  function            module     replaced
  //  patch3_2_call1       a.out     replaced into call2
  //  patch3_2_call2       a.out       called
  //
  //  patch3_2_call3       a.out     replaced into call4
  //  patch3_2_call4    libtestA       called
  //
  //  patch3_2_call5a   libtestA     replaced into call5b
  //  patch3_2_call5b   libtestB       called
  //
  //  patch3_2_call6    libtestA     replaced into call7
  //  patch3_2_call7       a.out       called
 * #Notes:
 */

#include "test_lib.h"
#include "patchapi_comp.h"

using Dyninst::PatchAPI::Instrumenter;
using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::FuncModMap;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::ReplaceFuncCommand;

class patch3_2_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
  PatchFunction* findFunc(const char* name);
};

extern "C" DLLEXPORT TestMutator* patch3_2_factory() {
  return new patch3_2_Mutator();
}

PatchFunction* patch3_2_Mutator::findFunc(const char* name) {
  PatchFunction* func = findFunction(name);
  if (func == NULL) {
    logerror("**Failed patch3_2 (func replacement)\n");
    logerror("  Cannot find function %s \n", name);
    return NULL;
  }
  logerror("- function %s found!\n", func->name().c_str());
  return func;
}

bool verify(FuncModMap& func_mod_map, PatchFunction* f1, PatchFunction* f2) {
  if (func_mod_map[f1] != f2) {
    logerror("**Failed patch3_2 (func replacement)\n");
    logerror(" replacement for function %s should be %s\n",
             f1->name().c_str(), f2->name().c_str());
    return false;
  }
  return true;
}

test_results_t patch3_2_Mutator::executeTest() {
  loadLibrary("libtestpatch1");
  loadLibrary("libtestpatch2");

  PatchFunction* pfunc = findFunc("patch3_2_func");
  if (pfunc == NULL) return FAILED;

  PatchFunction* call1 = findFunc("patch3_2_call1");
  if (call1 == NULL) return FAILED;

  PatchFunction* call2 = findFunc("patch3_2_call2");
  if (call2 == NULL) return FAILED;

  PatchFunction* call3 = findFunc("patch3_2_call3");
  if (call3 == NULL) return FAILED;

  PatchFunction* call4 = findFunc("patch3_2_call4");
  if (call4 == NULL) return FAILED;

  PatchFunction* call5a = findFunc("patch3_2_call5a");
  if (call5a == NULL) return FAILED;

  PatchFunction* call5b = findFunc("patch3_2_call5b");
  if (call5b == NULL) return FAILED;

  PatchFunction* call6 = findFunc("patch3_2_call6");
  if (call6 == NULL) return FAILED;

  PatchFunction* call7 = findFunc("patch3_2_call7");
  if (call7 == NULL) return FAILED;

  /* Step 1: replace function */
  Patcher patcher(mgr_);
  ReplaceFuncCommand::Ptr c1_to_2;   // a.out -> a.out
  ReplaceFuncCommand::Ptr c3_to_4;   // a.out -> lib
  ReplaceFuncCommand::Ptr c5a_to_5b; // lib -> lib
  ReplaceFuncCommand::Ptr c6_to_7;   // lib -> a.out
  c1_to_2 = ReplaceFuncCommand::create(mgr_, call1, call2);
  c3_to_4 = ReplaceFuncCommand::create(mgr_, call3, call4);
  c5a_to_5b = ReplaceFuncCommand::create(mgr_, call5a, call5b);
  c6_to_7 = ReplaceFuncCommand::create(mgr_, call6, call7);
  patcher.add(c1_to_2);
  patcher.add(c3_to_4);
  patcher.add(c5a_to_5b);
  patcher.add(c6_to_7);
  patcher.commit();

  /* Step 2: verify */
  InstrumenterPtr inst = mgr_->instrumenter();
  FuncModMap& mod_map = inst->funcRepMap();
  if (!verify(mod_map, call1, call2)) return FAILED;
  if (!verify(mod_map, call3, call4)) return FAILED;
  if (!verify(mod_map, call5a, call5b)) return FAILED;
  if (!verify(mod_map, call6, call7)) return FAILED;

  return PASSED;
}
