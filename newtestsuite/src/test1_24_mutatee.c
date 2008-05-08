#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_24_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_24_globalVariable1[100];
int test1_24_globalVariable2 = 53;
int test1_24_globalVariable3;
int test1_24_globalVariable4 = 83;
int test1_24_globalVariable5;

/* to hold values from local array */
int test1_24_globalVariable6;
int test1_24_globalVariable7;

/* for 2-d arrays - array is not square and we avoid using diagonal elements
 *    to make sure we test address computation
 */
int test1_24_globalVariable8[10][15];
int test1_24_globalVariable9;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void verifyValue24(const char *name, int *a, int index, int value);
static void verifyScalarValue24(const char *name, int a, int value);
static void call24_2();
static int verifyValue(const char *name, int *a, int index, int value,
			const char *tst, const char *tn);

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int dummy;
static int foo;

static int test_failed = FALSE;

/* Function definitions follow */

/*
 * Test #24 - arrary variables
 */
int test1_24_mutatee() {
    int i, j;
    int retval;

#if !defined(sparc_sun_solaris2_4) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_nt4_0) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(os_linux) /* Use OS #define instead of platform - Greg */

    logerror("Skipped test #24 (array variables)\n");
    logerror("\t- not implemented on this platform\n");
    test_passes(testname); /* Test "passes" */
    retval = 0;
#else

    /* passedTest[24] = TRUE; */


    for (i=0; i < 100; i++) test1_24_globalVariable1[i] = 2400000;
    test1_24_globalVariable1[79] = 2400003;
    test1_24_globalVariable1[83] = 2400004;

    for (i=0; i < 10; i++) {
	for (j=0; j < 15; j++) {
	    test1_24_globalVariable8[i][j] = 2400010;
	}
    }
    test1_24_globalVariable8[7][9] = 2400012;

    /* inst code we put into this function:
     *  At Call:
     *     test1_24_globalVariable1[1] = 2400001
     *     test1_24_globalVariable1[test1_24_globalVariable2] = 2400002
     *	   test1_24_globalVariable3 = test1_24_globalVariable1[79]
     *	   test1_24_globalVariable5 = test1_24_globalVariable1[test1_24_globalVariable4]
     *     localVariable24_1[1] = 2400001
     *     localVariable24_1[test1_24_globalVariable2] = 2400002
     *	   test1_24_globalVariable8[2][3] = 2400011
     *	   test1_24_globalVariable6 = localVariable24_1[79]
     *	   test1_24_globalVariable7 = localVariable24_1[test1_24_globalVariable4]
     */
    test1_24_call1();

    for (i=0; i < 100; i++) {
	if (i == 1) {
	    /* 1st element should be modified by the snippet (constant index) */
	    verifyValue24("test1_24_globalVariable1", test1_24_globalVariable1, 1, 2400001);
	} else if (i == 53) {
	    /* 53rd element should be modified by the snippet (variable index) */
	    verifyValue24("test1_24_globalVariable1", test1_24_globalVariable1, 53, 2400002);
	} else if (i == 79) {
	    /* 79th element was modified by us  */
	    verifyValue24("test1_24_globalVariable1", test1_24_globalVariable1, 79, 2400003);
	} else if (i == 83) {
	    /* 83rd element was modified by us  */
	    verifyValue24("test1_24_globalVariable1", test1_24_globalVariable1, 83, 2400004);
	} else if (test1_24_globalVariable1[i] != 2400000) {
	    /* rest should still be the original value */
	    verifyValue24("test1_24_globalVariable1", test1_24_globalVariable1, i, 2400000);
	}
    }

    verifyScalarValue24("test1_24_globalVariable3", test1_24_globalVariable3, 2400003);
    verifyScalarValue24("test1_24_globalVariable5", test1_24_globalVariable5, 2400004);

    /* now for the two elements read from the local variable */
    verifyScalarValue24("test1_24_globalVariable6", test1_24_globalVariable6, 2400007);
    verifyScalarValue24("test1_24_globalVariable7", test1_24_globalVariable7, 2400008);

    /* verify 2-d element use */
    verifyScalarValue24("test1_24_globalVariable8[2][3]", test1_24_globalVariable8[2][3],
	 2400011);
    verifyScalarValue24("test1_24_globalVariable9", test1_24_globalVariable9, 2400012);

    if (!test_failed) {
      logerror("Passed test #24 (array variables)\n");
      test_passes(testname);
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }

#endif
    return retval;
}

void verifyValue24(const char *name, int *a, int index, int value) {
  if (!verifyValue(name, a, index, value, "test1_24", "array variables")) {
    test_failed = TRUE;
  }
}

void verifyScalarValue24(const char *name, int a, int value) {
  if (!verifyScalarValue(name, a, value, "test1_24", "array variables")) {
    test_failed = TRUE;
  }
}

/*
 * Verify that a passed array has the correct value in the passed element.
 *
 */
int verifyValue(const char *name, int *a, int index, int value,
                 const char *tst, const char *tn)
{
    if (a[index] != value) {
	if (!test_failed) {
	  logerror("**Failed** test %s (%s)\n", tst, tn);
	}
	logerror("  %s[%d] = %d, not %d\n", 
		name, index, a[index], value);
	return FALSE;
    } else {
      return TRUE;
    }
}

void call24_2() {
}

void test1_24_call1() {
#ifdef sparc_sun_solaris2_4
  unsigned i=0; /* hack to prevent g++'s optimizer making func uninstr'uble */
#else
  unsigned i;
#endif

    int localVariable24_1[100];

    for (i=0; i < 100; i++) localVariable24_1[i] = 2400000;

    localVariable24_1[79] = 2400007;
    localVariable24_1[83] = 2400008;

    call24_2();

    verifyValue24("localVariable24_1", localVariable24_1, 1, 2400005);
    verifyValue24("localVariable24_1", localVariable24_1, 53, 2400006);
}
