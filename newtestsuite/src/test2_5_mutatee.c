#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test2_5_spinning = 1;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/* This test does nothing but mark that it succeeded */
int test2_5_mutatee() {
  /* fprintf(stderr, "Running test2_5_mutatee()\n"); */
  while (test2_5_spinning) {
    /* Do nothing */
  }
  test_passes("test2_5");
  return 0; /* No error */
}
