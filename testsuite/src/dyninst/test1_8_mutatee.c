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

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test1_8_func1(int p1, int p2, int p3, int p4, int p5, int p6, int p7,
		   int p8, int p9, int p10);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_8_globalVariable1 = 1;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/*
 * Test #8 - preserve registers - expr
 *	Verify the complex AST expressions do not clobber application
 *	paramter registers.
 */
int test1_8_func1(int p1, int p2, int p3, int p4, int p5, int p6, int p7,
		  int p8, int p9, int p10) {
  int retval;

  dprintf("test1_8_func1 (...) called\n");

  if (test1_8_globalVariable1 != 676) {
    logerror("**Failed** test #8 (preserve registers - expr)\n");
    logerror("    Global variable has wrong value (instrumentation not called?)\n");
    retval = -1; /* Test failed */
  } else if ((p1 == 1) && (p2 == 2) && (p3 == 3) && (p4 == 4) && (p5 == 5) &&
      (p6 == 6) && (p7 == 7) && (p8 == 8) && (p9 == 9) && (p10 == 10))  {
    logerror("Passed test #8 (preserve registers - expr)\n");
    retval = 0; /* Test passed */
  } else {
    logerror("**Failed** test #8 (preserve registers - expr )\n");
    if (p1 != 1)  logerror("    parameter #1 is %d not 1\n", p1);
    if (p2 != 2)  logerror("    parameter #2 is %d not 2\n", p2);
    if (p3 != 3)  logerror("    parameter #3 is %d not 3\n", p3);
    if (p4 != 4)  logerror("    parameter #4 is %d not 4\n", p4);
    if (p5 != 5)  logerror("    parameter #5 is %d not 5\n", p5);
    if (p6 != 6)  logerror("    parameter #6 is %d not 6\n", p6);
    if (p7 != 7)  logerror("    parameter #7 is %d not 7\n", p7);
    if (p8 != 8)  logerror("    parameter #8 is %d not 8\n", p8);
    if (p9 != 9)  logerror("    parameter #9 is %d not 9\n", p9);
    if (p10 != 10) logerror("    parameter #10 is %d not 10\n", p10);
    retval = -1; /* Test failed */
  }
  return retval;
}

int test1_8_mutatee() {
  int retval = test1_8_func1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  if (retval == 0) {
    test_passes(testname);
  }
  return retval;
}
