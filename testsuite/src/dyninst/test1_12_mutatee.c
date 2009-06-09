#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_12_func2();
void test1_12_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static volatile int globalVariable12_1 = 0;

/* Function definitions follow */

/*
 * Test #12 - insert/remove and malloc/free
 */	

int test1_12_mutatee() {
  int retval;
  /* kludge = 1; /* Here so that the following function call isn't the first
                    instruction */
  test1_12_func2();
  stop_process_();
  test1_12_func2();
  if (globalVariable12_1 == 1) {
    logerror("Passed test #12 (insert/remove and malloc/free)\n");
    test_passes(testname);
    retval = 0; /* Test passed */
  } else {
    logerror("**Failed test #12 (insert/remove and malloc/free)\n");
    logerror("ZANDY: #12 failed because globalVariable12_1 == %d\n",
	     globalVariable12_1);
    retval = -1;
  }
  return retval;
}

void test1_12_func2() {
  DUMMY_FN_BODY;
}

void test1_12_call1() {
  globalVariable12_1++;
}
