#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_14_func2();
void test1_14_func3();
void test1_14_call1();

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

static int globalVariable14_1 = 0;
static int globalVariable14_2 = 0;

/* Function definitions follow */

/*
 * Test #14 - replace function call
 */
void test1_14_func2() {
    globalVariable14_1 = 2;
}

void test1_14_func3() {
    globalVariable14_2 = 1;
}

int test1_14_func1() {
  int retval;
  /* kludge = 1; /* Here so that the following function call isn't the first
		    instruction */

    test1_14_func2();

    test1_14_func3();

    if (globalVariable14_1 == 1 && globalVariable14_2 == 0) {
        logerror("Passed test #14 (replace/remove function call)\n");
	retval = 0; /* Test passed */
    } else {
        logerror("**Failed test #14 (replace/remove function call)\n");
	if (globalVariable14_1 != 1)
    	    logerror("    call to test1_14_func2() was not replaced\n");
	if (globalVariable14_2 != 0)
	    logerror("    call to test1_14_func3() was not removed\n");
	retval = -1; /* Test failed */
    }
    return retval;
}

int test1_14_mutatee() {
  if (test1_14_func1() == 0) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}

void test1_14_call1() {
    globalVariable14_1 = 1;
}
