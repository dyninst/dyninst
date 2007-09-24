// Implementation file for the TestMutator class (all stubs, really)

#include "test_lib.h"

#include "TestMutator.h"

// All the constructor does is set the instance fields to NULL
TestMutator::TestMutator() {
  appThread = NULL;
  appImage = NULL;
}

bool TestMutator::hasCustomExecutionPath() {
  return false;
}

// Standard setup; this does the setup for the "simple" class of tests.
// "Simple" tests are tests that only perform any processing in the preExecution
// stage.  Most just do some lookup in the mutatee and then optionally insert
// instrumentation and let the mutatee do its thing.
test_results_t TestMutator::setup(ParameterDict &param) {
  bool useAttach = param["useAttach"]->getInt();
  appThread = (BPatch_thread *)(param["appThread"]->getPtr());

  // Read the program's image and get an associated image object
  appImage = appThread->getImage();

  if ( useAttach ) {
    if ( ! signalAttached(appThread, appImage) ) {
      return FAILED;
    }
  }

  return PASSED;
}

// This method should only be run in test objects that provide a custom
// execution path.
test_results_t TestMutator::execute() {
  return SKIPPED;
}

test_results_t TestMutator::preExecution() {
  if (!hasCustomExecutionPath()) {
    logstatus("**WARNING** Using default preExecution()\n");
  }
  return PASSED;
}

test_results_t TestMutator::inExecution() {
  return PASSED;
}

// I'd like this method to look into the mutatee and determine whether or not it
// passed, rather than depending on the mutatee to do the right thing.
test_results_t TestMutator::postExecution() {
  return PASSED;
}

test_results_t TestMutator::teardown() {
  return PASSED;
}

TestMutator *TestMutator_factory() {
  return new TestMutator();
}
