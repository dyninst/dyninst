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
 * Test #21 - findFunction in module
 */
int test1_21_mutatee()
{
  /* Nothing for the mutatee to do in this test (findFunction in module) */
#if defined(os_aix) \
 || defined(os_osf) \
 || defined(os_solaris) \
 || defined(os_linux) \
 || defined(os_windows)
     logerror("Passed test #21 (findFunction in module)\n");
     test_passes(testname);
#else
    logerror("Skipped test #21 (findFunction in module)\n");
    logerror("\t- not implemented on this platform\n");
    /* TODO Find a way to signal test skipped instead of faking a pass */
    test_passes(testname);
#endif
    return 0; /* Test "passes" */
}
