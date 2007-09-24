#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_30_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

unsigned long test1_30_globalVariable3 = 0;
unsigned long test1_30_globalVariable4 = 0;
unsigned long test1_30_globalVariable5 = 0;
unsigned long test1_30_globalVariable6 = 0;
/* See also test1_30_globalVariable7, defined below */
unsigned long test1_30_globalVariable8 = 0;
unsigned long test1_30_globalVariable9 = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func30_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* variables to keep the base addr and last addr of test1_30_call1 */
static unsigned long globalVariable30_1 = 0;
static unsigned long globalVariable30_2 = 0;

/* Function definitions follow */

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(alpha_dec_osf4_0) \
 || defined(ia64_unknown_linux2_4)

/* this function has to be only 1 line for test30 to pass */
/* these two lines have to be together otherwise test30 will fail */
unsigned long test1_30_globalVariable7 = __LINE__;
void test1_30_call1(){ globalVariable30_1 = __LINE__; globalVariable30_2 = (unsigned long)test1_30_call1;}

#endif

int test1_30_mutatee() {
  /* kludge = 1; /* Here so that the following function call isn't the first
                    instruction */
  int passed, retval;

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) \
 || defined(i386_unknown_nt4_0) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(alpha_dec_osf4_0)
    
    func30_2();

    passed = !test1_30_globalVariable3 ||
#if defined(rs6000_ibm_aix4_1)
             ((test1_30_globalVariable8 <= test1_30_globalVariable3) &&
#else
	     ((globalVariable30_2 <= test1_30_globalVariable3) &&
#endif
	      (test1_30_globalVariable3 <= test1_30_globalVariable9));

    if (!passed){
    	logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return -1; /* Test failed */
    }

    passed = !test1_30_globalVariable4 ||
#if defined(rs6000_ibm_aix4_1)
	     ((test1_30_globalVariable8 <= test1_30_globalVariable4) &&
#else
	     ((globalVariable30_2 <= test1_30_globalVariable4) &&
#endif
	      (test1_30_globalVariable4 <= test1_30_globalVariable9));

    if (!passed) {
    	logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return -1; /* Test failed */
    }

    passed = !test1_30_globalVariable5 ||
#if defined(rs6000_ibm_aix4_1)
	      ((test1_30_globalVariable8 <= test1_30_globalVariable5) &&
#else
	      ((globalVariable30_2 <= test1_30_globalVariable5) &&
#endif
	       (test1_30_globalVariable5 <= test1_30_globalVariable9));

    if (!passed) {
      logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
      logerror("gv30_5 = %lu, gv30_2 = %lu, gv30_9 = %lu\n", test1_30_globalVariable5,
	       globalVariable30_2, test1_30_globalVariable9);
      return -1; /* Test failed */
    }

    passed = !test1_30_globalVariable6 ||
	      (globalVariable30_1 == test1_30_globalVariable6);
    if (!passed) {
    	logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	logerror("gv30_6 = %lu, gv30_1 = %lu, should be equal and nonzero!\n", test1_30_globalVariable6,
	       globalVariable30_1);
	return -1; /* Test failed */
    }
	       
    logerror("Passed test #30 (line information)\n");
    retval = 0; /* Test passed */
    test_passes(testname);
#else
    logerror("Skipped test #30 (line information)\n");
    logerror("\t- not implemented on this platform\n");
    test_passes(testname);
    retval = 0; /* Test "passed" */
#endif
    return retval;
}

void func30_2() {
    DUMMY_FN_BODY;
}
