/*
 * #Name: patch1_2
 * #Desc: Mutator Side - Insert Snippet Order
 * #Dep:
 * #Notes:
 */

#include "test_lib.h"
#include "patchapi_comp.h"

class patch1_2_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch1_2_factory() {
  return new patch1_2_Mutator();
}

test_results_t patch1_2_Mutator::executeTest() {
  //std::cerr << "before pssed\n";
  return PASSED;
}
