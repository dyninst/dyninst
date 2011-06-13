#include "ParameterDict.h"
#include "test_lib.h"
#include "ResumeLog.h"

#include "patchapi_comp.h"
#include "CodeObject.h"
#include "AddrSpace.h"
#include "PatchMgr.h"
#include "PatchObject.h"

using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::ParseAPI::CodeObject;

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

}

test_results_t PatchApiComponent::program_setup(ParameterDict &params) {

  return PASSED;
}

test_results_t PatchApiComponent::program_teardown(ParameterDict &params) {

  return PASSED;
}

test_results_t PatchApiComponent::group_setup(RunGroup *group,
                                             ParameterDict &params) {

  return PASSED;
}

test_results_t PatchApiComponent::group_teardown(RunGroup *group,
                                                ParameterDict &params) {

  return PASSED;
}

test_results_t PatchApiComponent::test_setup(TestInfo *test,
                                         ParameterDict &parms) {

  return PASSED;
}

test_results_t PatchApiComponent::test_teardown(TestInfo *test,
                                         ParameterDict &parms) {

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

}

PatchApiMutator::~PatchApiMutator() {
}

test_results_t PatchApiMutator::setup(ParameterDict &param) {

  return PASSED;
}

/* Utilities for testing PatchAPI */

PatchMgrPtr makePatchMgr(CodeObject* co) {
  PatchObject* obj = PatchObject::create(co, 0);
  AddrSpacePtr as = AddrSpace::create(obj);
  PatchMgrPtr mgr = PatchMgr::create(as);
  return mgr;
}

