#include "mutatee_util.h"

#define do_dyninst_breakpoint() stop_process_()

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_stack_1_func1();
void test_stack_1_func2();
void test_stack_1_func3();

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

static int globalVariable1_1 = 0;

/* Function definitions follow */

void test_stack_1_func3() {
    globalVariable1_1++;
    do_dyninst_breakpoint();
    /* fprintf(stderr, "[%s:%u] - Mutatee continued after breakpoint\n", __FILE__, __LINE__); /*DEBUG*/
}

void test_stack_1_func2() {
    globalVariable1_1++;
    test_stack_1_func3();
}

void test_stack_1_func1() {
    globalVariable1_1++;
    test_stack_1_func2();
}

int test_stack_1_mutatee() {
  test_stack_1_func1();
  /* If the mutatee passed the test, set passedTest[0] to TRUE, and return 0 */
  test_passes(testname);
  /* fprintf(stderr, "[%s:%u] - Mutatee function exiting\n", __FILE__, __LINE__); /*DEBUG*/
  return 0; /* Return code for this mutatee is not checked */
}
