#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_23_call1();
void test1_23_call2();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_23_shadowVariable1 = 2300010;
int test1_23_shadowVariable2 = 2300020;
int test1_23_globalVariable1 = 2300000;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void verifyScalarValue23(const char *name, int a, int value);

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalShadowVariable23_1 = (int)0xdeadbeef;
static int globalShadowVariable23_2 = (int)0xdeadbeef;
static int test_failed = FALSE;

/* Function definitions follow */

/*
 * Test #23 - local variables
 */
int test1_23_mutatee() {
/* We don't support MIPS any more, so we don't need to check for it
 * #if defined(mips_sgi_irix6_4)
 *   logerror("Skipped test #23 (local variables)\n");
 *   logerror("\t- not implemented on this platform\n");
 *   passedTest[23] = TRUE;
 * #else
 */
  int retval;

  test1_23_call1();

  if (!test_failed) {
    logerror("Passed test #23 (local variables)\n");
    test_passes(testname);
    retval = 0; /* Test passed */
  } else {
    retval = -1; /* Test failed */
  }
  return retval;
/* #endif */
}

void verifyScalarValue23(const char *name, int a, int value)
{
  if (!verifyScalarValue(name, a, value, "test1_23", "local variables")) {
    test_failed = TRUE;
  }
}

void test1_23_call2()
{
    /* copy shadowed global variables to visible global variables to permit
     *    checking their values
     */
    globalShadowVariable23_1 = test1_23_shadowVariable1;
    globalShadowVariable23_2 = test1_23_shadowVariable2;
}

void test1_23_call1()
{
    int localVariable23_1 = 2300019;
    int test1_23_shadowVariable1 = 2300011;
    int test1_23_shadowVariable2 = 2300021;

    test1_23_call2();			/* place to manipulate local variables */

    /* passedTest[23] = TRUE; */

    /* snippet didn't update local variable */
    verifyScalarValue23("localVariable23_1", localVariable23_1, 2300001);

    /* did snippet update shadow variable (in the global scope) */
    verifyScalarValue23("globalShadowVariable23_1", globalShadowVariable23_1,
	2300010);

    /* did snippet correctly update shadow variable test1_23_call2 */
    verifyScalarValue23("test1_23_shadowVariable1", test1_23_shadowVariable1, 2300012);

    /* did snippet correctly update shadow variable via global
       scope in test1_23_call2 */
    verifyScalarValue23("test1_23_shadowVariable2", test1_23_shadowVariable2, 2300021);

    /* did snippet correctly update shadow variable via global
       scope in test1_23_call2 */
    verifyScalarValue23("globalShadowVariable23_2", globalShadowVariable23_2,
	2300023);

    /* did snippet correctly read local variable in test1_23_call2 */
    verifyScalarValue23("test1_23_globalVariable1", test1_23_globalVariable1, 2300001);
    dprintf("Leaving test1_23_call1...\n");
}
