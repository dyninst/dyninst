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

static static_test test4;
static int passed = 0;

int static_test::count = 0;
static int local_file_var = 3;

/* Function definitions follow */

void static_test::pass()
{
  passed = 1;
   logerror("Passed test #4 (static member)\n");
}

void static_test::func_cpp()
{
   static_test obj1, obj2;

   if ((obj1.call_cpp()+1) != obj2.call_cpp()) {
     logerror("**Failed** test #4 (static member)\n");
     logerror("    C++ objects of the same class have different static members\n");
   }
}

int test5_4_mutatee() {
#if !defined(os_solaris) && !defined(os_linux) && !defined(os_windows)

    logerror("Skipped test #4 (static member)\n");
    logerror("\t- not implemented on this platform\n");
    return 0;

#else
  test4.func_cpp();
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
