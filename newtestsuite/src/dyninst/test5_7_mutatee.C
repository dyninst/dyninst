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
static template_test test7;

/* Function definitions follow */

template <class T> T sample_template <T>::content()
{
   T  ret = item;
   return (ret);
}

// Why do we have this section?  We don't run this test on AIX...
#ifdef rs6000_ibm_aix4_1
/* xlC's static libC has strangely undefined symbols, so just fake them ... */
// BUG(?) I don't know if these will work when declared as static.  Need to
// test
static int SOMClassClassData;
static int SOMObjectClassData;
#else
/* xlC also doesn't like these, so just skip them ... */
template class sample_template <int>;
template class sample_template <char>;
template class sample_template <double>;
#endif

void template_test::func_cpp()
{
  int int7 = 7;
  char char7 = 'c';
  sample_template <int> item7_1(int7);
  sample_template <char> item7_2(char7);
 
  item7_1.content();
  item7_2.content();
}

void test5_7_passed()
{
  passed = 1;
  logerror("Passed test #7 (template)\n");
}

int test5_7_mutatee() {
#if !defined(os_solaris) && !defined(os_linux) && !defined(os_windows)
    logerror("Skipped test #7 (template)\n");
    logerror("\t- not implemented on this platform\n");
#else
  test7.func_cpp();
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
