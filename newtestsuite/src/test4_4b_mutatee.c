#include <stdlib.h>

#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test4_4_func2();
void test4_4_func4();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test4_4_global1 = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

void test4_4_func4()
{
    dprintf("in test4b test4_4_func4\n");

    test4_4_global1 = 4000003;
}

void test4_4_func2()
{
    dprintf("in test4b test4_4_func2\n");

    /* a call to test4_4_func4 gets hooked onto the exit of this */
}

int test4_4b_mutatee() {
#ifndef i386_unknown_nt4_0
    dprintf("in test4b func4_1\n");

    test4_4_func2();

    exit(getpid());
    return -1; /* Not reachable */
#else
    return 0; /* Test skipped.. */
#endif
}
