#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_34_func2();

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

/*
 * Nested loops.
 */

int test1_34_mutatee() {
  /* The only possible failures occur in the mutator, and if it fails, the
   * mutatee will never be run.  So if the mutatee is run, just return success.
   */
  logerror( "Passed test #34 (loop information)\n" );
  test_passes(testname);
  return 0; /* Test passed */
}

void test1_34_func2() {
    int i, j, k;

    /* There are four loops in total. */
    for (i = 0; i < 10; i++) { /* Contains two loops. */
	dprintf("i = %d\n", i);

	for (j = 0; j < 10; j++) { /* Contains one loop. */
	    dprintf("j = %d\n", j);

	    k = 0;
	    while (k < 10) {
		dprintf("k = %d\n", k);
		k++;
	    }
	}

	do {
	    j++;
	    dprintf("j = %d\n", j);

	} while (j < 10);
    }
}
