#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_sparc_3_func();
void test_sparc_3_call();

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

// I hope this works here..
// moved after func4 to make sure it appears to be "outside" func3
asm("           .text");
asm("label3:");
asm("		retl");
asm("		nop");
asm("		call abort");
asm("		nop");

//
// use call; load %07 with current PC 
//
asm("		.stabs  \"test_sparc_3_func:F(0,1)\",36,0,16,test_sparc_3_func");
asm("		.global test_sparc_3_func");
asm("		.type   test_sparc_3_func,#function");
asm("test_sparc_3_func:");
asm("		save %sp, -112, %sp");
asm("		nop");
asm("		nop");
asm("		call label3");		// external call to set pc into o7
asm("		nop");
asm("		call call0");
asm("		nop");
asm("		ret");
asm("		restore");

void call0()
{
}

void test_sparc_3_call()
{
  passed = 1;
  logerror("\nPassed test #3\n");
}

int test_sparc_3_mutatee() {
  test_sparc_3_func();
  if (1 == passed) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}
