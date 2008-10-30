#ifndef TEST_INFO_NEW_H
#define TEST_INFO_NEW_H

#include <vector>
#include <string>
#include <map>

#include "TestData.h"
#include "test_results.h"

// Avoid stupid circular dependency issue..
class TestMutator;
class ComponentTester;

#define NUM_RUNSTATES 7
typedef enum {
   program_setup_rs = 0,
   group_setup_rs,
   group_teardown_rs,
   test_init_rs,
   test_setup_rs,
   test_execute_rs,
   test_teardown_rs
} test_runstate_t;

class TestInfo {
public:
  const char *name;
  const char *mutator_name;
  const char *soname;
  const char *label;
  TestMutator *mutator;
  // This test has been explicitly disabled, probably by the resumelog system
  bool disabled;
  bool enabled;
  unsigned int index;
  
  test_results_t results[NUM_RUNSTATES];
  bool result_reported;
  
  
  TESTLIB_DLL_EXPORT TestInfo(unsigned int i, const char *iname, const char *mrname,
	   const char *isoname, const char *ilabel);
  TESTLIB_DLL_EXPORT ~TestInfo();
};

class Module;

class RunGroup {
public:
  char *mutatee;
  start_state_t state;
  create_mode_t useAttach;
  bool customExecution;
  bool selfStart;
  unsigned int index;
  std::vector<TestInfo *> tests;
  bool disabled;
  Module *mod;

  TESTLIB_DLL_EXPORT RunGroup(char *mutatee_name, start_state_t state_init,
                              create_mode_t attach_init, bool ex, TestInfo *test_init, char *modname_);
  TESTLIB_DLL_EXPORT RunGroup(char *mutatee_name, start_state_t state_init,
                              create_mode_t attach_init, bool ex, char *modname_);
  TESTLIB_DLL_EXPORT ~RunGroup();
};

class Module {
   bool creation_error;
   static std::map<std::string, Module *> allmods;

   Module(std::string name_);
   ComponentTester *loadModuleLibrary();
   void *libhandle;
public:
   std::string name;
   ComponentTester *tester;
   std::vector<RunGroup *> groups;

   static bool registerGroupInModule(std::string modname, RunGroup *group);
   static void getAllModules(std::vector<Module *> &mods);
};

//extern std::vector<RunGroup *> tests;

extern void initialize_mutatees(std::vector<RunGroup *> &tests);

#endif // !defined(TEST_INFO_NEW_H)
