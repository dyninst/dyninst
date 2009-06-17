#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_sparc_4_func();
void test_sparc_4_call();

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
// use call; load %07 with current PC 
//
asm("		.stabs  \"test_sparc_4_func:F(0,1)\",36,0,16,test_sparc_4_func");
asm("		.global test_sparc_4_func");
asm("		.type   test_sparc_4_func,#function");
asm("           .text");
asm("test_sparc_4_func:");
asm("		save %sp, -112, %sp");
asm("		nop");
asm("		call .+8");		// call to set pc into o7
asm("		nop");
asm("		nop");
asm("           ret");
asm("           restore");

void call0()
{
}

void test_sparc_4_call()
{
  passed = 1;
  logerror("\nPassed test #4\n");
}

int test_sparc_4_mutatee() {
  test_sparc_4_func();
  if (1 == passed) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    
    return -1; /* Test failed */
  }
}
