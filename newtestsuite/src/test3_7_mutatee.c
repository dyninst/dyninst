#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test3_7_call1();

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

unsigned int test7counter = 0;

/* Function definitions follow */

/* 
 * Test #1 - just run indefinitely (to be killed by mutator)
 */
int test3_7_mutatee()
{
     dprintf("Mutatee spinning.\n");
     while (1);
     return -1; /* Don't think I should ever get here */
}

void test3_7_call1() {
  test7counter++;
}

