#include "mutatee_util.h"

#define do_dyninst_breakpoint() stop_process_()

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_stack_3_func1();
void test_stack_3_func2();
void test_stack_3_func3();
void test_stack_3_func4();

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

void test_stack_3_func4() {
  /* Do nothing */ ;
}

void test_stack_3_func3() {
  do_dyninst_breakpoint();
} /* end test_stack_3_func3() */

void test_stack_3_func2() {
  /* This function will be instrumented to call test_stack_3_func3, which
     stops the mutatee and allows the mutator to walk the stack. */
	   
  /* This is to give us a third place to instrument. */
  test_stack_3_func4();
} /* end test_stack_3_func2() */
	
void test_stack_3_func1() {
  /* Stop myself.  The mutator will instrument test_stack_3_func2() at this point. */
  do_dyninst_breakpoint();
	
  /* This function will be instrumented. */
  test_stack_3_func2();
} /* end test_stack_3_func1() */

/* skeleton test doesn't do anything besides say that it passed */
int test_stack_3_mutatee() {
  test_stack_3_func1();
  return 0; /* Return code is not checked */
}
