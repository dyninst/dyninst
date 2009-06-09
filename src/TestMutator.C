// Implementation file for the TestMutator class (all stubs, really)

#include "test_lib.h"

bool TestMutator::hasCustomExecutionPath() {
  return false;
}

TestMutator::TestMutator() 
{
}

TestMutator::~TestMutator() 
{
}

test_results_t TestMutator::setup(ParameterDict &param) {
   return PASSED;
}

// This method should only be run in test objects that provide a custom
// execution path.
test_results_t TestMutator::executeTest() {
  return SKIPPED;
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
