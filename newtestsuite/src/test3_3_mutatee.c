#include <assert.h>

#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test3_3_call1(int arg1, int arg2);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

volatile int test3_3_ret = (int)0xdeadbeef;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

volatile int dummy = 1;

/* Function definitions follow */

void test3_3_call1(int arg1, int arg2)
{
     dprintf("test3_3_call1() called with arg1=%d,arg2=%d\n", arg1, arg2);
}

/*
 * Test #3 - call a function which should be instrumented to set the 
 *     global variable test3ret to a value (by the mutator).
 */
int test3_3_mutatee() {
     FILE *fp;
     char filename[80];

     sprintf(filename, "test3.out.%d", (int)getpid());
     fp = fopen(filename, "w");
     assert(fp);
     fprintf(fp, "%d\n", test3_3_ret);
     fclose(fp);
     return 0; /* No error on this end */
}
