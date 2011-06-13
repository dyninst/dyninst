/*
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
  //std::cerr << "before pssed\n";
  return PASSED;
}
