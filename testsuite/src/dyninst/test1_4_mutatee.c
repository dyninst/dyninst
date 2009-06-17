#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test1_4_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_4_globalVariable4_1 = 41;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func4_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

static void func4_2() {
  dprintf("func4_1 () called\n");
}

int test1_4_func1() {
  int retval;

  /* kludge = 1; /* Here so that the following function call isn't the first
                  * instruction
		  */

  func4_2();

  if (test1_4_globalVariable4_1 == 41) {
    logerror("**Failed** test #4 (sequence)\n");
    logerror("    none of the items were executed\n");
    retval = -1; /* Test failed */
  } else if (test1_4_globalVariable4_1 == 42) {
    logerror("**Failed** test #4 (sequence)\n");
    logerror("    first item was the last (or only) one to execute\n");
    retval = -1; /* Test failed */
  } else if (test1_4_globalVariable4_1 == 43) {
    logerror("Passed test #4 (sequence)\n");
    retval = 0; /* Test passed */
  }
  return retval;
}

/*
 * Start of Test #4 - sequence
 *	Run two expressions and verify correct ordering.
 */
int test1_4_mutatee() {
  if (test1_4_func1()) {
    return -1;
  } else {
    test_passes(testname);
    return 0;
  }
}
