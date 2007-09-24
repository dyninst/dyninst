#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_1_call1_1();
void test1_1_func1_1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func1_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalVariable1_1 = 0;

/* Function definitions follow */

void test1_1_call1_1() {
  dprintf("call1() called - setting globalVariable1_1 = 11\n");
  globalVariable1_1 = 11;
}

static void func1_2() {
  dprintf("func1_2 () called\n");
}

void test1_1_func1_1() {
  dprintf("Value of globalVariable1_1 is %d.\n", globalVariable1_1);

  func1_2();

  dprintf("Value of globalVariable1_1 is now %d.\n", globalVariable1_1);

  if (globalVariable1_1 == 11) {
    logerror("\nPassed test #1 (zero arg function call)\n");
    /* test_passes(testname); */
  } else {
    logerror("\n**Failed** test #1 (zero arg function call)\n");
  }
  flushOutputLog();
}

int test1_1_mutatee() {
  test1_1_func1_1();
  if (11 == globalVariable1_1) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}
