#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

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

/* Function definitions follow */

void test1_41_func1() {
  DUMMY_FN_BODY;
}

/* This function doesn't really need to do anything.  All we're testing is
 * whether parsing returns the same values on multiple mutatee loads.. */
int test1_41_mutatee() {
  test1_41_func1();
  return -1; /* This function should never return */
}
