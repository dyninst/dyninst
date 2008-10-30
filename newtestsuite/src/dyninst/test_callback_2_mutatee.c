#include "mutatee_util.h"
#include "test12.h"


/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test_callback_2_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

volatile int test_callback_2_idle = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int call7_2(int x)
{
  int i;
  int y = x;
  for (i = 0; i < 0xfff; ++i) 
    y += i;
  
  return y;
}

int test_callback_2_call1() 
{
  int x = 0;
  int z = 0;
  int i;

  for (i = 0; i < TEST7_NUMCALLS; ++i) {
    z += call7_2(x); 
  }
  return z;
}

void func7_1()
{
  /* This is a simple single threaded scenario for user defined callback
   * testing.  The entry, exit, and call points of test_callback_2_call1 are
   * instrumented with messaging functions.
   */
  int x = 0;
  x = test_callback_2_call1();
  while (test_callback_2_idle == 0); /* Loop while *not* idle? */
}

int test_callback_2_mutatee() {
  func7_1();
  return 0; /* Return code is not checked */
}
