#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_39_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_39_passed = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/* Test #39 (refex function search) */
int test1_39_mutatee() {
#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) \
 || defined(mips_sgi_irix6_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(os_linux) /* Use OS #define instead of platform - Greg */

    /* The only possible failures occur in the mutator. */
  if (1 == test1_39_passed) {
    logerror( "Passed test #39 (regex function search)\n" );
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
#else
   /*  no regex for windows */
   logerror("Skipped test #39 (regex function search)\n");
   logerror("\t- not implemented on this platform\n");
   test_passes(testname);
   return 0; /* Test "passed" */
#endif
}

void test1_39_func1() {
  DUMMY_FN_BODY;
}
