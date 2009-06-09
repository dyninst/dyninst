#ifndef TEST_MUTATOR_H
#define TEST_MUTATOR_H

#include "test_lib.h"

// Base class for the mutator part of a test
class TestMutator {
public: 
  TESTLIB_DLL_EXPORT TestMutator();
  TESTLIB_DLL_EXPORT virtual bool hasCustomExecutionPath();
  TESTLIB_DLL_EXPORT virtual test_results_t setup(ParameterDict &param);
  TESTLIB_DLL_EXPORT virtual test_results_t executeTest();
  TESTLIB_DLL_EXPORT virtual test_results_t postExecution();
  TESTLIB_DLL_EXPORT virtual test_results_t teardown();
  TESTLIB_DLL_EXPORT virtual ~TestMutator();
};
extern "C" {
TESTLIB_DLL_EXPORT TestMutator *TestMutator_factory();
}

#endif /* !TEST_MUTATOR_H */
