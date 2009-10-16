#include "mutatee_util.h"
#include <stdlib.h>

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_write_param_call1(long p1, long p2, long p3, long p4, long p5, long p6, long p7, long p8);
void test_write_param_call2(long p1, long p2, long p3, long p4, long p5, long p6, long p7, long p8);
int test_write_param_call3();

void test_write_param_func1();

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
static int test_passed = 1;

/* Function definitions follow */

void test_write_param_call1(long p1, long p2, long p3, long p4, long p5, 
                            long p6, long p7, long p8)
{
   if (p1 != 1) {
      logerror("parameter p1 is incorrect in call1\n");
      test_passed = 0;
   }
   if (p2 != 2) {
      logerror("parameter p2 is incorrect in call1\n");
      test_passed = 0;
   }
   if (p3 != 3) {
      logerror("parameter p3 is incorrect in call1\n");
      test_passed = 0;
   }
   if (p4 != 4) {
      logerror("parameter p4 is incorrect in call1\n");
      test_passed = 0;
   }
   if (p5 != 5) {
      logerror("parameter p5 is incorrect in call1\n");
      test_passed = 0;
   }
   if (p6 != 6) {
      logerror("parameter p6 is incorrect in call1\n");
      test_passed = 0;
   }
   if (p7 != 7) {
      logerror("parameter p7 is incorrect in call1\n");
      test_passed = 0;
   }      
   if (p8 != 8) {
      logerror("parameter p8 is incorrect in call1\n");
      test_passed = 0;
   }      
}

void test_write_param_call2(long p1, long p2, long p3, long p4, long p5, long p6, long p7, long p8)
{
   if (p1 != 11) {
      logerror("parameter p1 is incorrect in call2\n");
      test_passed = 0;
   }
   if (p2 != 12) {
      logerror("parameter p2 is incorrect in call2\n");
      test_passed = 0;
   }
   if (p3 != 13) {
      logerror("parameter p3 is incorrect in call2\n");
      test_passed = 0;
   }
   if (p4 != 14) {
      logerror("parameter p4 is incorrect in call2\n");
      test_passed = 0;
   }
   if (p5 != 15) {
      logerror("parameter p5 is incorrect in call2\n");
      test_passed = 0;
   }
   if (p6 != 16) {
      logerror("parameter p6 is incorrect in call2\n");
      test_passed = 0;
   }
   if (p7 != 17) {
      logerror("parameter p7 is incorrect in call2\n");
      test_passed = 0;
   }      
   if (p8 != 18) {
      logerror("parameter p8 is incorrect in call2\n");
      test_passed = 0;
   }
}

int test_write_param_call3()
{
   return 0;
}

int test_write_param_call4()
{
   return 0;
}

int test_write_param_func() {
   test_write_param_call1(0, 0, 0, 0, 0, 0, 0, 0);
   test_write_param_call2(0, 0, 0, 0, 0, 0, 0, 0);

   if (test_write_param_call3() != 20) {
      test_passed = 0;
      logerror("Return value for call3 was incorrect\n");
   }

   if (test_write_param_call4() != 30) {
      test_passed = 0;
      logerror("Return value for call4 was incorrect\n");
   }

   if (!test_passed) {
      logerror("test_write_param failed");
      return -1;
   }
   return 0;
}

int test_write_param_mutatee() {
  int result;
  result = test_write_param_func();
  if (result != -1) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}
