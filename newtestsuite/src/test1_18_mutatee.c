#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test1_18_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_18_globalVariable1 = 42;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/*
 * Test #18 - read/write a variable in the mutatee
 */
int test1_18_func1() {
  int retval;
  if (test1_18_globalVariable1 == 17) {
    logerror("Passed test #18 (read/write a variable in the mutatee)\n");
    retval = 0; /* Test passed */
  } else {
    logerror("**Failed test #18 (read/write a variable in the mutatee)\n");
    if (test1_18_globalVariable1 == 42) {
      logerror("    test1_18_globalVariable1 still contains 42 (probably it was not written to)\n");
    } else {
      logerror("    test1_18_globalVariable1 contained %d, not 17 as expected\n",
	       test1_18_globalVariable1);
    }
    retval = -1; /* Test failed */
  }
  return retval;
}

int test1_18_mutatee() {
  if (test1_18_func1() == 0) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}
