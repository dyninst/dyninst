#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_10_func1();
void test1_10_call1();
void test1_10_call2();
void test1_10_call3();

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

static int globalVariable10_1 = 0;
static int globalVariable10_2 = 0;
static int globalVariable10_3 = 0;
static int globalVariable10_4 = 0;

static int passed = 0;

/* Function definitions follow */

/*
 * Test #10 - insert snippet order
 *	Verify that a snippet are inserted in the requested order.  We insert
 *	one snippet and then request two more to be inserted.  One before
 *	the first snippet and one after it.
 */
int test1_10_mutatee() {
  test1_10_func1();

  if (passed) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}

void test1_10_func1() {
  if ((globalVariable10_1 == 1) && (globalVariable10_2 == 1) &&
      (globalVariable10_3 == 1) && (globalVariable10_4 == 3)) {
    logerror("Passed test #10 (insert snippet order)\n");
    passed = 1;
  } else {
    logerror("** Failed test #10 (insert snippet order)\n");
    if (!globalVariable10_1)
      logerror("    test1_10_call1 was not called first\n");
    if (!globalVariable10_2)
      logerror("    test1_10_call2 was not called second\n");
    if (!globalVariable10_3)
      logerror("    test1_10_call3 was not called third\n");
    passed = 0;
  }
}

void test1_10_call1() {
  if (globalVariable10_4 == 0) {
    globalVariable10_4 = 1;
    globalVariable10_1 = 1;
  }
}


void test1_10_call2() {
  if (globalVariable10_4 == 1) {
    globalVariable10_4 = 2;
    globalVariable10_2 = 1;
  }
}

void test1_10_call3() {
  if (globalVariable10_4 == 2) {
    globalVariable10_4 = 3;
    globalVariable10_3 = 1;
  }
}
