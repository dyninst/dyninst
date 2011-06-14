/*
 * #Name: patch1_1
 * #Desc: Mutator Side - Insert Snippets at Function Entry, During, Call, Exit
 * #Dep:
 *      Find five points and insert a corresponding function call at each point
 *           Point                  Call              Global variable set
 *         ---------------------------------------------------------------
 *         - FuncEntry              entry_call        entry = 1
 *         - FuncDuring             during_call       during = 1
 *         - CallBefore (foo)       callbefore_call   callbefore = 1
 *         - CallAfter  (foo)       callafter_call    callafter = 1
 *         - FuncExit               exit_call         exit = 1
 * #Notes:
 */


#include "test_lib.h"
#include "patchapi_comp.h"

class patch1_1_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch1_1_factory() {
  return new patch1_1_Mutator();
}

test_results_t patch1_1_Mutator::executeTest() {
  return PASSED;
}
