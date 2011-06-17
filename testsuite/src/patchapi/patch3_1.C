/*
 * #Name: patch3_1
 * #Desc: Mutator Side : function call replacement / removal
 * #Dep:
 *      1, replace the call to patch3_1_call1 with patch3_1_call1_1
 * #Notes:
 */

#include "test_lib.h"
#include "patchapi_comp.h"

class patch3_1_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch3_1_factory() {
  return new patch3_1_Mutator();
}

test_results_t patch3_1_Mutator::executeTest() {
  return PASSED;
}
