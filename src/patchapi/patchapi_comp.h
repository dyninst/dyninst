#ifndef TESTSUITE_SRC_PATCHAPI_COMP_H_
#define TESTSUITE_SRC_PATCHAPI_COMP_H_

#include "TestMutator.h"
#include <iostream>

#include "CodeObject.h"
#include "Command.h"
#include "PatchMgr.h"
#include "PatchObject.h"

#define test_cerr std::cerr

using namespace Dyninst::ParseAPI;

class COMPLIB_DLL_EXPORT PatchApiMutator : public TestMutator {
 public:
  PatchApiMutator();
  virtual test_results_t setup(ParameterDict &param);
  virtual ~PatchApiMutator();

 protected:
  create_mode_t runmode_;
  Dyninst::PatchAPI::PatchMgrPtr mgr_;
  Dyninst::ParseAPI::SymtabCodeSource* sts_lib_;
  Dyninst::ParseAPI::CodeObject* co_lib_;

  Dyninst::PatchAPI::PatchMgrPtr makePatchMgr(CodeObject* co);
  Dyninst::PatchAPI::PatchFunction* findFunction(const char* name);
  void loadLibrary(char* libname);
};

#endif /* TESTSUITE_SRC_PATCHAPI_COMP_H_ */
