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
/* Include files */
#include <assert.h>
#include <string.h>
#include "test1.h"

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_2_func2_1();
void test1_2_call2_1(int arg1, int arg2, char *arg3, void *arg4);

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

volatile int passed = FALSE;
static volatile int call2_1_called = FALSE;

/* Function definitions follow */

void test1_2_func2_1() {
  dprintf("func2_1 () called\n");
}

void test1_2_call2_1(int arg1, int arg2, char *arg3, void *arg4)
{
   unsigned long val = TEST_VAL; //val variable works around bug in pgCC
   call2_1_called = TRUE;
   assert(TEST_PTR_SIZE == sizeof(void *));
   if ((arg1 == 1) && (arg2 == 2) && (!strcmp(arg3, "testString2_1")) &&
       (arg4 == (void *) val)) {
      logerror("Passed test #2 (four parameter function)\n");
      passed = TRUE;
   } else {
      logerror("**Failed** test #2 (four parameter function)\n");
      if (arg1 != 1)
         logerror("    arg1 = %d, should be 1\n", arg1);
      if (arg2 != 2)
         logerror("    arg2 = %d, should be 2\n", arg2);
      if (strcmp(arg3, "testString2_1"))
         logerror("    arg3 = %s, should be \"testString2_1\"\n", arg3);
      if (arg4 != TEST_PTR)
         logerror("    arg4 = %p, should be %p\n", arg4, TEST_PTR);
   }
}

/* Set up is easy here, just call func2_1 */
int test1_2_mutatee() {
  test1_2_func2_1();
  if (passed) {
    test_passes(testname);
    return 0; /* No error */
  } else {
    if (!call2_1_called) {
      logerror("**Failed** test #2 (four parameter function)\n");
      logerror("    instrumentation not called\n");
    }
    return -1; /* Test failed */
  }
}
