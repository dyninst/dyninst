#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_15_func2();
void test1_15_func3();
void test1_15_func4();
void test1_15_call1();
void test1_15_call3();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void call15_2();
static void check15result(const char *varname, int value, int expected,
			  const char *errstr, int *failed);

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalVariable15_1 = 0;
static int globalVariable15_2 = 0;
static int globalVariable15_3 = 0;
static int globalVariable15_4 = 0;

/* Function definitions follow */

/*
 * Test #15 - setMutationsActive
 */
void check15result(const char *varname, int value, int expected,
                   const char *errstr, int *failed)
{
    if (value != expected) {
	if (!*failed)
	    logerror("**failed test #15 (setMutationsActive)\n");
	*failed = TRUE;

	logerror("    %s = %d %s\n", varname, value, errstr);
    }		
}


void test1_15_func2()
{
    DUMMY_FN_BODY;
}

void test1_15_func3()
{
    globalVariable15_3 = 100;
    /* increment a dummy variable to keep alpha code generator from assuming
       too many free registers on the call into test1_15_func3. jkh 3/7/00 */
    globalVariable15_4++;
}

void test1_15_func4()
{
    /* int localkludge = 1;
    /* kludge = localkludge; /* Here so that the following function call isn't
                                the first instruction */

    test1_15_func3();
}

int test1_15_mutatee() {
    int failed = FALSE;
    int retval;

    test1_15_func2();
    check15result("globalVariable15_1", globalVariable15_1, 1,
		  "after first call to instrumented function", &failed);

#if 0
    // Deprecated pending rewrite of function relocation
#if defined(sparc_sun_sunos4_1_3) \
 || defined(sparc_sun_solaris2_4)
    /* Test a function that makes a system call (is a special case on Sparc) */
    access(".", R_OK);
    check15result("globalVariable15_2", globalVariable15_2, 2,
		  "after first call to instrumented function", &failed);
#endif
#endif

    test1_15_func4();
    check15result("globalVariable15_3", globalVariable15_3, 1,
		  "after first call to instrumented function", &failed);

    /***********************************************************/

    stop_process_();

    test1_15_func2();
    check15result("globalVariable15_1", globalVariable15_1, 1,
		  "after second call to instrumented function", &failed);
#if 0
#if defined(sparc_sun_sunos4_1_3) \
 || defined(sparc_sun_solaris2_4)
    access(".", R_OK);
    check15result("globalVariable15_2", globalVariable15_2, 2,
		  "after second call to instrumented function", &failed);
#endif
#endif

    test1_15_func4();
    check15result("globalVariable15_3", globalVariable15_3, 100,
		  "after second call to instrumented function", &failed);

    /***********************************************************/

    stop_process_();

    test1_15_func2();
    check15result("globalVariable15_1", globalVariable15_1, 2,
		  "after third call to instrumented function", &failed);
#if 0
#if defined(sparc_sun_sunos4_1_3) \
 || defined(sparc_sun_solaris2_4)
    access(".", R_OK);
    check15result("globalVariable15_2", globalVariable15_2, 4,
		  "after third call to instrumented function", &failed);
#endif
#endif
    test1_15_func4();
    check15result("globalVariable15_3", globalVariable15_3, 101,
		  "after third call to instrumented function", &failed);

    if (!failed) {
        logerror("Passed test #15 (setMutationsActive)\n");
	test_passes("test1_15");
	retval = 0; /* Test passed */
    } else {
        retval = -1; /* Test failed */
    }
    return retval;
}

void test1_15_call1() {
    globalVariable15_1++;
}

/* It looks like this function is unused. */
void call15_2() {
    globalVariable15_2++;
}

void test1_15_call3() {
    globalVariable15_3++;
}
