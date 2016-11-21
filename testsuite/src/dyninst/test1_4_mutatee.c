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

int test1_4_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_4_globalVariable4_1 = 41;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func4_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

static void func4_2() {
  dprintf("func4_1 () called\n");
}

int test1_4_func1() {
  int retval;

  func4_2();

  if (test1_4_globalVariable4_1 == 41) {
    logerror("**Failed** test #4 (sequence)\n");
    logerror("    none of the items were executed\n");
    retval = -1; /* Test failed */
  } else if (test1_4_globalVariable4_1 == 42) {
    logerror("**Failed** test #4 (sequence)\n");
    logerror("    first item was the last (or only) one to execute\n");
    retval = -1; /* Test failed */
  } else if (test1_4_globalVariable4_1 == 43) {
    logerror("Passed test #4 (sequence)\n");
    retval = 0; /* Test passed */
  }
  return retval;
}

/*
 * Start of Test #4 - sequence
 *	Run two expressions and verify correct ordering.
 */
int test1_4_mutatee() {
  if (test1_4_func1()) {
    return -1;
  } else {
    test_passes(testname);
    return 0;
  }
}
