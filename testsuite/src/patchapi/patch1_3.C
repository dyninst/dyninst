/*
 * #Name: patch1_3
 * #Desc: Mutator Side - insert at instruction before
 * #Dep:
 * #Notes:
 */

#include "test_lib.h"
#include "patchapi_comp.h"

class patch1_3_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch1_3_factory() {
  return new patch1_3_Mutator();
}

test_results_t patch1_3_Mutator::executeTest() {
  //std::cerr << "before pssed\n";
  return PASSED;
}
