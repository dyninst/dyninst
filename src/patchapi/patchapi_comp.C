#include "ParameterDict.h"
#include "test_lib.h"
#include "ResumeLog.h"

#include "patchapi_comp.h"

class PatchApiComponent : public ComponentTester {
  private:
    // ParamPtr bpatch_ptr_;
    std::string err_msg_;
  public:
    PatchApiComponent();
    virtual test_results_t program_setup(ParameterDict &params);
    virtual test_results_t program_teardown(ParameterDict &params);
    virtual test_results_t group_setup(RunGroup *group, ParameterDict &params);
    virtual test_results_t group_teardown(RunGroup *group, ParameterDict &params);
    virtual test_results_t test_setup(TestInfo *test, ParameterDict &parms);
    virtual test_results_t test_teardown(TestInfo *test, ParameterDict &parms);
    virtual std::string getLastErrorMsg();
    virtual ~PatchApiComponent();
};

PatchApiComponent::PatchApiComponent() {
  test_cerr << "PatchApiComponent\n";
}

test_results_t PatchApiComponent::program_setup(ParameterDict &params) {
  test_cerr << "program_setup\n";
  return PASSED;
}

test_results_t PatchApiComponent::program_teardown(ParameterDict &params) {
  test_cerr << "program_teardown\n";
  return PASSED;
}

test_results_t PatchApiComponent::group_setup(RunGroup *group,
                                             ParameterDict &params) {
  test_cerr << "group_setup\n";
  return PASSED;
}

test_results_t PatchApiComponent::group_teardown(RunGroup *group,
                                                ParameterDict &params) {
  test_cerr << "group_teardown\n";
  return PASSED;
}

test_results_t PatchApiComponent::test_setup(TestInfo *test,
                                         ParameterDict &parms) {
  test_cerr << "test_setup\n";
  return PASSED;
}

test_results_t PatchApiComponent::test_teardown(TestInfo *test,
                                         ParameterDict &parms) {
  test_cerr << "test_teardown\n";
  return PASSED;
}

std::string PatchApiComponent::getLastErrorMsg() {
  return err_msg_;
}

extern "C" {
  COMPLIB_DLL_EXPORT ComponentTester *componentTesterFactory();
}

COMPLIB_DLL_EXPORT ComponentTester *componentTesterFactory() {
  return new PatchApiComponent();
}

PatchApiComponent::~PatchApiComponent() {
}

PatchApiMutator::PatchApiMutator() {
  test_cerr<< "PatchApiMutator\n";
}

PatchApiMutator::~PatchApiMutator() {
}

test_results_t PatchApiMutator::setup(ParameterDict &param) {
  test_cerr << "PatchApiMutator::setup\n";
  return PASSED;
}

