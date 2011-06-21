#include "ParameterDict.h"
#include "test_lib.h"
#include "ResumeLog.h"

#include "Command.h"
#include "PatchCFG.h"
#include "PatchMgr.h"
#include "AddrSpace.h"
#include "CodeObject.h"
#include "PatchObject.h"
#include "patchapi_comp.h"

using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::PatcherPtr;
using Dyninst::PatchAPI::Patcher;

class PatchApiComponent : public ComponentTester {
  private:
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
  private:
    SymtabCodeSource* sts_;
    CodeObject* co_;
    ParamPtr code_obj_;
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
  sts_ = new SymtabCodeSource(const_cast<char*>(group->mutatee));
  co_ = new CodeObject(sts_);
  co_->parse();
  code_obj_.setPtr(co_);
  params["code_obj"] = &code_obj_;
  return PASSED;
}

test_results_t PatchApiComponent::group_teardown(RunGroup *group,
                                                ParameterDict &params) {
  delete sts_;
  delete co_;
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

PatchApiMutator::PatchApiMutator() : co_lib_(NULL), sts_lib_(NULL) {
}

PatchApiMutator::~PatchApiMutator() {
	if (co_lib_) delete co_lib_;
	if (sts_lib_) delete sts_lib_;
}

test_results_t PatchApiMutator::setup(ParameterDict &param) {
  CodeObject* co = (CodeObject*)param["code_obj"]->getPtr();
  mgr_ = makePatchMgr(co);
  return PASSED;
}

/* Utilities for testing PatchAPI */

PatchMgrPtr PatchApiMutator::makePatchMgr(CodeObject* co) {
  PatchObject* obj = PatchObject::create(co, 0);
  AddrSpacePtr as = AddrSpace::create(obj);
  PatchMgrPtr mgr = PatchMgr::create(as);
  return mgr;
}

PatchFunction* PatchApiMutator::findFunction(const char* name) {
  AddrSpacePtr as = mgr_->as();
  AddrSpace::ObjSet& obj_set = as->objSet();

  for (AddrSpace::ObjSet::iterator i = obj_set.begin(); i != obj_set.end(); i++) {
    PatchObject* obj = *i;
    CodeObject* co = obj->co();
    CodeObject::funclist& funcs = co->funcs();
    for (CodeObject::funclist::iterator j = funcs.begin(); j != funcs.end(); j++) {
      Function* fun = *j;
      if (0 == fun->name().compare(name)) return obj->getFunc(fun);
    }
  }

  return NULL;
}

void PatchApiMutator::loadLibrary(char* libname) {
  char fullname[128];
  char lib_name[128];
  sprintf(lib_name, "%s", libname);
  CodeObject* lib = mgr_->as()->getFirstObject()->co();
  SymtabCodeSource* scs = static_cast<SymtabCodeSource*>(lib->cs());
  bool isStatic = scs->getSymtabObject()->isStaticBinary();

  int pointer_size = 0;
#if defined(arch_x86_64_test) || defined(ppc64_linux_test)
  pointer_size = sizeof(void*);
#endif

  addLibArchExt(lib_name, 127, pointer_size, isStatic);

  sprintf(fullname, "./%s", lib_name);
  sts_lib_ = new SymtabCodeSource(fullname);
  co_lib_ = new CodeObject(sts_lib_);
  co_lib_->parse();

  PatchObject* obj_lib = PatchObject::create(co_lib_, 0);
  mgr_->as()->loadObject(obj_lib);
  logerror("load %s\n", fullname);
}
