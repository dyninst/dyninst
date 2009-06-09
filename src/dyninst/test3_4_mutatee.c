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


/* 
 * Test #2 - just run in a busy wait loop and then exit.
 */
int test3_4_mutatee() {
     int i;

     for (i=0; i < 2000000; i++);
     dprintf("Mutatee exiting.\n");
     return 0; /* No special error conditions... */
}
