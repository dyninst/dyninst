#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_sparc_2_func();
void test_sparc_2_call();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void call0();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static volatile int passed = 0;

/* Function definitions follow */

//
// force call; move to %07 tail code function
//
asm("		.stabs  \"test_sparc_2_func:F(0,1)\",36,0,16,test_sparc_2_func");
asm("		.global test_sparc_2_func");
asm("		.type   test_sparc_2_func,#function");
asm("           .text");
asm("test_sparc_2_func:");
asm("		sethi %hi(call0),%l0");
asm("		or %l0,%lo(call0),%l0");
asm("		mov %o7, %l0");
asm("		call call0");
asm("		mov %l0, %o7");

void call0()
{
}

void test_sparc_2_call()
{
  passed = 1;
  logerror("\nPassed test #2\n");
}

int test_sparc_2_mutatee() {
  test_sparc_2_func();
  if (1 == passed) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}
