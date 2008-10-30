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

/* Function definitions follow */

arg_test test1; 

void arg_test::func_cpp()
{
#if !defined(os_solaris) && !defined(os_linux) && !defined(os_windows)
    logerror("Skipped test #1 (argument)\n");
    logerror("\t- not implemented on this platform\n");
    test_passes(testname);

#else

  int test = 1;
  int arg2 = 1;

  call_cpp(test, arg2);
#endif
}

void arg_test::arg_pass(int test)
{
   if (test != 1 || value != 5) {
     logerror("**Failed** test #1 (C++ argument pass)\n");
     logerror("    Passed in an incorrect parameter value:\n");
     logerror("    test = %d, this = %u, test1 = %u\n",
	      test, (void *)this, (void *)&test1);
//      cerr << "    test = " << test << ", this = " << (void *) this;
//       cerr << ", test1 = " << (void *) &test1;
      return;
   }
   logerror("Passed test #1 (C++ argument pass)\n");
   test_passes(testname);
}

void arg_test::dummy()
{
  DUMMY_FN_BODY;
}

void arg_test::call_cpp(const int arg1, int & arg2, int arg3)
{
   const int m = 8;
   int n = 6;
   int & reference = n;

   dummy(); // place to change the value of arg3 from CPP_DEFLT_ARG to 1 

   if ( 1 != arg3 ) {
     logerror("**Failed** test #1 (C++ argument pass)\n");
     logerror("    Default argument value is not changed\n");
   }

   if ( arg1 == arg2 )  arg2 = CPP_DEFLT_ARG;
}

int test5_1_mutatee() {
  test1.func_cpp();
  // FIXME Make sure the error reporting works
  // I need to have this guy call test_passes(testname) if the test passes..
  return 0; /* No error */
}
