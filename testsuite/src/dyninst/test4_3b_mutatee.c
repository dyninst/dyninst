#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test4_3_func1();
void test4_3_func2();

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

/* Function definitions follow */

int globalVariable3_1 = 0xdeadbeef;

void test4_3_func2() {
  dprintf("in test4_3_func2\n");
  globalVariable3_1 = 3000002;
}

void test4_3_func1() {
  dprintf("in test4_3_func1\n");
}  

int test4_3b_mutatee() {
  test4_3_func1();
  return 0; /* No error, right? */
}
