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
static exception_test test6;

/* Function definitions follow */

void sample_exception::response()
{
   DUMMY_FN_BODY;
}


void test5_6_passed(int arg)
{
   if ( arg == 6 ) {
     passed = 1;
     logerror("Passed test #6 (exception)\n");
   } else {
     logerror("**Failed** test #6 (exception)\n");
   }
}


void exception_test::call_cpp()
{
  throw sample_exception();
}


void exception_test::func_cpp()
{
  try {
    int testno = 6;
    call_cpp();
  }
  catch ( sample_exception & ex) {
    ex.response();
    return;
  }
  catch ( ... ) {
    logerror("**Failed** test #6 (exception)\n");
    logerror("    Does not catch appropriate exception\n");
    throw;
  }
  logerror("Missed proper exception\n");
}

int test5_6_mutatee() {
#if defined(os_linux) && defined(arch_x86) && !defined(arch_x86_64)
  test6.func_cpp();
  // FIXME Make sure the error reporting works
  // I need to have this guy call test_passes(testname) if the test passes..
  if (1 == passed) {
    // Test passed
    test_passes(testname);
    return 0;
  } else {
    // Test failed
    return -1;
  }
#else
  logerror("Skipped test #6 (exception)\n");
  logerror("\t- not implemented on this platform\n");
  return 0;
#endif
}
