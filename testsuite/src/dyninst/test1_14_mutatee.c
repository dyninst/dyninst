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

void test1_14_func2();
void test1_14_func3();
void test1_14_call1();

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

static int globalVariable14_1 = 0;
static int globalVariable14_2 = 0;
static int globalVariable14_3 = 0;

/* Function definitions follow */

/*
 * Test #14 - replace function call
 */
void test1_14_func2() {
    globalVariable14_1 = 2;
}

void test1_14_func3() {
    globalVariable14_2 = 1;
}

void test1_14_func4(int *var) {
    *var = 3;
}

int test1_14_func1() {
  int retval;
  /* kludge = 1; */ /* Here so that the following function call isn't the first
                 instruction */

    test1_14_func2();

    test1_14_func3();

    test1_14_func4(&globalVariable14_3);

    if ((globalVariable14_1 == 1) &&
        (globalVariable14_2 == 0) && 
        (globalVariable14_3 == 2)) {
        logerror("Passed test #14 (replace/remove function call)\n");
	retval = 0; /* Test passed */
    } else {
        logerror("**Failed test #14 (replace/remove function call)\n");
	if (globalVariable14_1 != 1)
    	    logerror("    call to test1_14_func2() was not replaced\n");
	if (globalVariable14_2 != 0)
	    logerror("    call to test1_14_func3() was not removed\n");
        if (globalVariable14_3 != 2) 
	  logerror("    call to test1_14_func4() was not inter-module replaced: %d instead of 2\n", globalVariable14_3);
	retval = -1; /* Test failed */
    }
    return retval;
}

int test1_14_mutatee() {

  if (test1_14_func1() == 0) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}

void test1_14_call1() {
    globalVariable14_1 = 1;
}
