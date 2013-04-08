/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
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
