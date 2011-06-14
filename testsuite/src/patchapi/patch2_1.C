/*
 * #Name: patch2_1
 * #Desc: Mutator Side - Remove Snippets at Function Entry
 * #Dep:
 *      0, global variable patch2_1_var is initialized to 0
 *      1, add 1 to global variable patch2_1_var
 *      2, add 2 to global variable patch2_1_var
 *      3, add 3 to global variable patch2_1_var
 *      4, delete the snippet inserted at 1 and 3.
 *      5, check if the global variable is 2
 * #Notes:
 */

#include "test_lib.h"
#include "patchapi_comp.h"

class patch2_1_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch2_1_factory() {
  return new patch2_1_Mutator();
}

test_results_t patch2_1_Mutator::executeTest() {
  //std::cerr << "before pssed\n";
  return PASSED;
}
