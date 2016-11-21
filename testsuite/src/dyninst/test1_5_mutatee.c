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

void test1_5_func2();
int test1_5_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_5_globalVariable5_1 = 51;
int test1_5_globalVariable5_2 = 51;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test1_5_func1() {
  int retval;
  test1_5_func2();

  if ((test1_5_globalVariable5_1 == 51)
      && (test1_5_globalVariable5_2 == 53)) {
    logerror("Passed test #5 (if w.o. else)\n");
    retval = 0; /* Test passed */
  } else {
    logerror("**Failed** test #5 (if w.o. else)\n");
    if (test1_5_globalVariable5_1 != 51) {
      logerror("    condition executed for false\n");
    }
    if (test1_5_globalVariable5_2 != 53) {
      logerror("    condition not executed for true\n");
    }
    retval = -1; /* Test failed */
  }
  return retval;
}

/*
 * Start of Test #5 - if w.o. else
 *	Execute two if statements, one true and one false.
 */
int test1_5_mutatee() {
  if (test1_5_func1()) {
    return -1; /* Test failed */
  } else {
    test_passes(testname);
    return 0; /* Test passed */
  }
}

void test1_5_func2() {
  dprintf("func5_1 () called\n");
}
