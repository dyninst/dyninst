#ifndef TEST_INFO_NEW_H
#define TEST_INFO_NEW_H

#include <vector>

#include "TestData.h"

// Avoid stupid circular dependency issue..
class TestMutator;

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

  TestInfo(unsigned int i, const char *iname, const char *mrname,
	   const char *isoname, const char *ilabel);
};

class RunGroup {
public:
  char *mutatee;
  start_state_t state;
  create_mode_t useAttach;
  bool customExecution;
  bool selfStart;
  unsigned int index;
  std::vector<TestInfo *> tests;

  RunGroup(char *mutatee_name, start_state_t state_init,
	   create_mode_t attach_init, bool ex, TestInfo *test_init);
  RunGroup(char *mutatee_name, start_state_t state_init,
	   create_mode_t attach_init, bool ex);
  ~RunGroup();
};

//extern std::vector<RunGroup *> tests;

extern void initialize_mutatees(std::vector<RunGroup *> &tests);

#endif // !defined(TEST_INFO_NEW_H)
