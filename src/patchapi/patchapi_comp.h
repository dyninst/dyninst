#ifndef TESTSUITE_SRC_PATCHAPI_COMP_H_
#define TESTSUITE_SRC_PATCHAPI_COMP_H_

#include "TestMutator.h"
#include <iostream>

#include "Command.h"

#define test_cerr std::cerr

class COMPLIB_DLL_EXPORT PatchApiMutator : public TestMutator {
 public:
  PatchApiMutator();
  virtual test_results_t setup(ParameterDict &param);
  virtual ~PatchApiMutator();

 protected:
  create_mode_t runmode_;
};

/* PatchAPI plugins */


#endif /* TESTSUITE_SRC_PATCHAPI_COMP_H_ */
