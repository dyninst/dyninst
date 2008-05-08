#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_sparc_1_func1();
void test_sparc_1_call1();

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
// force jmp %reg; nop tail code function
//
asm("		.align 8");
asm("		.stabs  \"test_sparc_1_func1:F(0,1)\",36,0,16,test_sparc_1_func1");
asm("		.global test_sparc_1_func1");
asm("		.type   test_sparc_1_func1,#function");
asm("           .text");
asm("test_sparc_1_func1:");
asm("		sethi   %hi(call0),%g1");
asm("		or      %g1,%lo(call0),%g1");
asm("		jmp     %g1");
asm("		nop	");

void call0()
{
}

void test_sparc_1_call1() {
  passed = 1;
  logerror("\nPassed test_sparc_1\n");
}

int test_sparc_1_mutatee() {
  test_sparc_1_func1();
  if (1 == passed) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}
