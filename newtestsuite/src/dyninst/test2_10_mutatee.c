#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test2_10_passed = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test2_10_mutatee() {
  /* All we do is check whether the mutator set test2_10_passed */
  if (test2_10_passed) {
    test_passes(testname);
    return 0; /* No error */
  } else {
    return -1; /* Error */
  }
}
