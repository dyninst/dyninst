#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test1_29_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_29_globalVariable1 = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test1_29_func1() {
    int passed, retval;
    passed = (test1_29_globalVariable1 == 1);

    if (passed) {
      logerror("Passed test #29 (BPatch_srcObj class)\n");
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }

    return retval;
}

int test1_29_mutatee() {
  if (test1_29_func1() == 0) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}
