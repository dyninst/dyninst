#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test2_8_passed = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/* skeleton test doesn't do anything besides say that it passed */
int test2_8_mutatee() {
  if (test2_8_passed) {
    test_passes("test2_8");
    return 0; /* No error */
  } else {
    return -1; /* Error */
  }
}
