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
static namespace_test test5;

static int local_file_var = 3;

/* Function definitions follow */

void namespace_test::func_cpp()
{
  int local_fn_var = local_file_var;

  class_variable = local_fn_var;

  if ( 1024 != ::CPP_DEFLT_ARG)
    logerror("::CPP_DEFLT_ARG init value wrong\n");
  if ( 0 != cpp_test_util::CPP_TEST_UTIL_VAR )
    logerror("cpp_test_util::CPP_TEST_UTIL_VAR int value wrong\n");
}

void namespace_test::pass() {
  passed = 1;
}

int test5_5_mutatee() {
#if !defined(os_solaris) && !defined(os_linux) && !defined(os_windows)

    logerror("Skipped test #5 (namespace)\n");
    logerror("\t- not implemented on this platform\n");
    return 0;

#else
  test5.func_cpp();
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
#endif
}
