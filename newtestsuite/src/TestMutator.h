#ifndef TEST_MUTATOR_H
#define TEST_MUTATOR_H

#include "test_lib.h"

// Base class for the mutator part of a test
class TestMutator {
public:
  BPatch_thread *appThread;
  // FIXME This field (appImage) probably isn't necessary.  It looks looks like
  // appImage is easily derivable from appThread.
  BPatch_image *appImage;

  TestMutator();
  virtual bool hasCustomExecutionPath();
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
  virtual test_results_t preExecution();
  virtual test_results_t inExecution();
  virtual test_results_t postExecution();
  virtual test_results_t teardown();
};
TestMutator *TestMutator_factory();

#endif /* !TEST_MUTATOR_H */
