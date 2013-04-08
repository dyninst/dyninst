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

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void amd64_7_arg_call_func();
void amd64_7_arg_call(int arg1, int arg2, int arg3, int arg4,
                     int arg5, int arg6, int arg7);

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
static volatile int seven_arg_func_called = FALSE;

/* Function definitions follow */

void amd64_7_arg_call_func() {
  dprintf("func2_1 () called\n");
}

void amd64_7_arg_call(int arg1, int arg2, int arg3, int arg4, int arg5,
                     int arg6, int arg7)
{
    seven_arg_func_called = TRUE;
   if ((arg1 == 0) && (arg2 == 1) && (arg3 == 2) && (arg4 == 3) &&
        (arg5 == 4) && (arg6 == 5) && (arg7 == 6)) {
      logerror("Passed (seven parameter function)\n");
      passed = TRUE;
   } else {
      logerror("**Failed** (seven parameter function)\n");
      if (arg1 != 0)
         logerror("    arg1 = %d, should be 0\n", arg1);
      if (arg2 != 1)
         logerror("    arg2 = %d, should be 1\n", arg2);
      if (arg3 != 2)
          logerror("    arg3 = %d, should be 2\n", arg3);
      if (arg4 != 3)
          logerror("    arg4 = %d, should be 3\n", arg4);
      if (arg5 != 4)
          logerror("    arg5 = %d, should be 4\n", arg5);
      if (arg6 != 5)
          logerror("    arg6 = %d, should be 5\n", arg6);
      if (arg7 != 6)
          logerror("    arg7 = %d, should be 6\n", arg7);
   }
}

/* Set up is easy here, just call func2_1 */
int amd64_7_arg_call_mutatee() {
  amd64_7_arg_call_func();
  if (passed) {
    test_passes(testname);
    return 0; /* No error */
  } else {
      if (!seven_arg_func_called) {
      logerror("**Failed** test (seven parameter function)\n");
      logerror("    instrumentation not called\n");
    }
    return -1; /* Test failed */
  }
}
