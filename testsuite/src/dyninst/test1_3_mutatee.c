#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_3_func3_1();
void test1_3_call3_1(int arg1, int arg2);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_3_globalVariable3_1 = 31;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int passed = FALSE;
static volatile int inst_called = FALSE;

/* Function definitions follow */

void test1_3_func3_1() {
  dprintf("func3_1 () called\n");
}

void test1_3_call3_1(int arg1, int arg2) {
  inst_called = TRUE;
  if ((arg1 == 31) && (arg2 == 32))  {
    logerror("Passed test #3 (passing variables to functions)\n");
    passed = TRUE;
  } else {
    logerror("**Failed** test #3 (passing variables to functions)\n");
    logerror("    arg1 = %d, should be 31\n", arg1);
    logerror("    arg2 = %d, should be 32\n", arg2);
  }
}

/* This test does nothing but mark that it succeeded */
int test1_3_mutatee() {
  test1_3_func3_1();
  if (passed) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    if (!inst_called) {
      logerror("**Failed** test #3 (passing variables to functions)\n");
      logerror("    instrumentation not called\n");
    }
    return -1; /* Test failed */
  }
}
