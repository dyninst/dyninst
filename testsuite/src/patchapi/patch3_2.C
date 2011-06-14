/*
 * #Name: patch3_2
 * #Desc: replace function
 * #Dep:
  //  Mutatee function replacement scheme:
  //  function            module     replaced    global or called?
  //  patch3_2_call1       a.out     replaced         1       global is the index
  //  patch3_2_call2       a.out       called         1       of the global variable
  //  patch3_2_call3       a.out     replaced         2       in test1.mutatee updated
  //  patch3_2_call4    libtestA       called         2       by the function
  //  patch3_2_call5a   libtestA     replaced         3
  //  patch3_2_call5b   libtestB       called         3
  //  patch3_2_call6    libtestA     replaced         4
  //  patch3_2_call7       a.out       called         4
 * #Notes:
 */

#include "test_lib.h"
#include "patchapi_comp.h"

class patch3_2_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch3_2_factory() {
  return new patch3_2_Mutator();
}

test_results_t patch3_2_Mutator::executeTest() {
  //std::cerr << "before pssed\n";
  return PASSED;
}
