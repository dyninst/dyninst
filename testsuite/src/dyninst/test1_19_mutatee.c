#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_19_call1();
void test1_19_call2();

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

static int globalVariable19_1 = (int)0xdeadbeef;
static int globalVariable19_2 = (int)0xdeadbeef;

#define MAGIC19_1 1900100
#define MAGIC19_2 1900200

/* Function definitions follow */

/*
 * Test #19 - oneTimeCode
 */
int test1_19_mutatee() {
    int retval = 0;

    stop_process_();

    if (globalVariable19_1 != MAGIC19_1) {
	logerror("**Failed test #19 (oneTimeCode)\n");
	logerror("    globalVariable19_1 contained %d, not %d as expected\n",
		 globalVariable19_1, MAGIC19_1);
	retval = -1; /* Test failed */
    }

    stop_process_();

    if (globalVariable19_2 == MAGIC19_2) {
        if (0 == retval) {
	  logerror("Passed test #19 (oneTimeCode)\n");
	  test_passes(testname);
	  retval = 0; /* Test passed */
	}
    } else {
	logerror("**Failed test #19 (oneTimeCode)\n");
	logerror("    globalVariable19_2 contained %d, not %d as expected\n",
		 globalVariable19_2, MAGIC19_2);
	retval = -1; /* Test failed */
    }
    return retval;
}

void test1_19_call1() {
    globalVariable19_1 = MAGIC19_1;
}

void test1_19_call2() {
    globalVariable19_2 = MAGIC19_2;
}
