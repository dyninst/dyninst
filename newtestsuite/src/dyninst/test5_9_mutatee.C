#include "cpp_test.h"
#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

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

static int passed = 0;
static derivation_test test9;

/* Function definitions follow */

void derivation_test::func_cpp()
{
  DUMMY_FN_BODY;
}

void test5_9_passed() {
  passed = 1;
  logerror("Passed test #9 (derivation)\n");
}

int test5_9_mutatee() {
#if !defined(os_solaris) && !defined(os_linux) && !defined(os_windows)
    logerror("Skipped test #8 (declaration)\n");
    logerror("\t- not implemented on this platform\n");
    return 0;
#else
  test9.func_cpp();
  if (1 == passed) {
    // Test passed
    test_passes(testname);
    return 0;
  } else {
    // Test failed
    logerror("**Failed** test5_9 (derivation)\n");
    logerror("    Instrumentation not called?\n");
    return -1;
  }
#endif
}
