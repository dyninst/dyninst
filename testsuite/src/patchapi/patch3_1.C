/*
 * #Name: patch3_1
 * #Desc: Mutator Side : function call replacement / removal
 * #Dep:
 *      1, replace the call to patch3_1_call1 with patch3_1_call1_1
 *      2, delete patch3_2_call2
 *      3, replace the call to patch3_1_call3 with patch3_1_call1_3
 * #Notes:
 */
 
#include "test_lib.h"
#include "patchapi_comp.h"

using Dyninst::PatchAPI::Instrumenter;
using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::CallModMap;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::ReplaceCallCommand;
using Dyninst::PatchAPI::RemoveCallCommand;

class patch3_1_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
  PatchFunction* findFunc(const char* name);
};

extern "C" DLLEXPORT TestMutator* patch3_1_factory() {
  return new patch3_1_Mutator();
}

PatchFunction* patch3_1_Mutator::findFunc(const char* name) {
  PatchFunction* func = findFunction(name);
  if (func == NULL) {
    logerror("**Failed patch3_1 (call replacement and removal)\n");
    logerror("  Cannot find function %s \n", name);
    return NULL;
  }
  logerror("- function %s found!\n", func->name().c_str());
  return func;
}

test_results_t patch3_1_Mutator::executeTest() {
  PatchFunction* pfunc = findFunc("patch3_1_func");
  if (pfunc == NULL) return FAILED;

  PatchFunction* call1_1 = findFunc("patch3_1_call1_1");
  if (call1_1 == NULL) return FAILED;

  PatchFunction* call1 = findFunc("patch3_1_call1");
  if (call1 == NULL) return FAILED;

  PatchFunction* call2 = findFunc("patch3_1_call2");
  if (call2 == NULL) return FAILED;

  PatchFunction* call3 = findFunc("patch3_1_call3");
  if (call3 == NULL) return FAILED;

  PatchFunction* call1_3 = NULL;

  /* Step 1: get call blocks */
  const PatchFunction::Blockset& callBlks = pfunc->getCallBlocks();
  if (callBlks.size() != 3) {
    logerror("**Failed patch3_1 (call replacement and removal)\n");
    logerror("  cannot find correct call blocks\n");
    return FAILED;
  }

  /* Step 2: replace / remove function call */
  Patcher patcher(mgr_);
  ReplaceCallCommand::Ptr c1;
  RemoveCallCommand::Ptr c2;
  ReplaceCallCommand::Ptr c3;

  PatchBlock* call_blk1;
  PatchBlock* call_blk2;
  PatchBlock* call_blk3;

  for (PatchFunction::Blockset::const_iterator i = callBlks.begin();
       i != callBlks.end(); i++) {
    PatchBlock* b = *i;
    PatchFunction* f = b->getCallee();
    if (f == call1) {
      logerror("  replace call to %s w/ %s\n", call1->name().c_str(), call1_1->name().c_str());
      c1 = ReplaceCallCommand::create(mgr_, b, call1_1, pfunc);
      patcher.add(c1);
      call_blk1 = b;
    } else if (f == call2) {
      logerror("  remove call to %s\n", call2->name().c_str());
      c2 = RemoveCallCommand::create(mgr_, b, pfunc);
      patcher.add(c2);
      call_blk2 = b;
    } else if (f == call3) {
      logerror("  replace call to %s\n", call3->name().c_str());

      // find call1_3 in libtestpatch1
      loadLibrary("libtestpatch1");
      call1_3 = findFunc("patch3_1_call3_1");
      if (call1_3 == NULL) return FAILED;

      c3 = ReplaceCallCommand::create(mgr_, b, call1_3, pfunc);
      patcher.add(c3);
      call_blk3 = b;
    } else {
      return FAILED;
    }
  }
  patcher.commit();

  /* Step 3: verify */
  InstrumenterPtr inst = mgr_->instrumenter();
  CallModMap& mod_map = inst->callModMap();
  if (mod_map[call_blk1][pfunc] != call1_1) {
    logerror("**Failed patch3_1 (call replacement and removal)\n");
    logerror(" replacement for call to %s should be %s\n",
             call_blk1->getCallee()->name().c_str(), call1_1->name().c_str());
    return FAILED;
  }
  if (mod_map[call_blk2][pfunc] != NULL) {
    logerror("**Failed patch3_1 (call replacement and removal)\n");
    logerror(" removal of call to %s fails\n", call_blk1->getCallee()->name().c_str());
    return FAILED;
  }
  if (mod_map[call_blk3][pfunc] != call1_3) {
    logerror("**Failed patch3_1 (call replacement and removal)\n");
    logerror(" replacement for call to %s should be %s\n",
            call_blk3->getCallee()->name().c_str(),
            call1_3->name().c_str());
    return FAILED;
  }

  return PASSED;
}
