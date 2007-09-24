#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_5_func2();
int test1_5_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_5_globalVariable5_1 = 51;
int test1_5_globalVariable5_2 = 51;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test1_5_func1() {
  int retval;
  test1_5_func2();

  if ((test1_5_globalVariable5_1 == 51)
      && (test1_5_globalVariable5_2 == 53)) {
    logerror("Passed test #5 (if w.o. else)\n");
    retval = 0; /* Test passed */
  } else {
    logerror("**Failed** test #5 (if w.o. else)\n");
    if (test1_5_globalVariable5_1 != 51) {
      logerror("    condition executed for false\n");
    }
    if (test1_5_globalVariable5_2 != 53) {
      logerror("    condition not executed for true\n");
    }
    retval = -1; /* Test failed */
  }
  return retval;
}

/*
 * Start of Test #5 - if w.o. else
 *	Execute two if statements, one true and one false.
 */
int test1_5_mutatee() {
  if (test1_5_func1()) {
    return -1; /* Test failed */
  } else {
    test_passes(testname);
    return 0; /* Test passed */
  }
}

void test1_5_func2() {
  dprintf("func5_1 () called\n");
}
