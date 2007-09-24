#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test2_7_passed = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test2_7_mutatee() {
#if defined(os_solaris) || defined(os_linux) || defined(os_aix) || defined(os_windows)
  if (test2_7_passed) {
    test_passes("test2_7");
    return 0; /* No error */
  } else {
    return -1; /* Test failed */
  }
#else
  return 0; /* Test "passed" (skipped) */
#endif
}
