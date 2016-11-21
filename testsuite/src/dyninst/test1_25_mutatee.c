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

void test1_25_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_25_globalVariable1;
int *test1_25_globalVariable2;	/* used to hold addres of test1_25_globalVariable1 */
int test1_25_globalVariable3;
int test1_25_globalVariable4;
int test1_25_globalVariable5;
int test1_25_globalVariable6;
int test1_25_globalVariable7;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int test_failed = 0;

/* Function definitions follow */

/*
 * Test #25 - unary operators
 */
int test1_25_mutatee() {
  int retval;

    test1_25_globalVariable1 = 25000001;
    test1_25_globalVariable2 = (int *) 25000002;
    test1_25_globalVariable3 = 25000003;
    test1_25_globalVariable4 = 25000004;
    test1_25_globalVariable5 = 25000005;
    test1_25_globalVariable6 = -25000006;
    test1_25_globalVariable7 = 25000007;

    /* inst code we put into this function:
     *  At Entry:
     *     test1_25_globalVariable2 = &test1_25_globalVariable1
     *     test1_25_globalVariable3 = *test1_25_globalVariable2
     *     test1_25_globalVariable5 = -test1_25_globalVariable4
     *     test1_25_globalVariable7 = -test1_25_globalVariable6
     */

    test1_25_call1();

    if ((int *) test1_25_globalVariable2 != &test1_25_globalVariable1) {
	if (!test_failed) {
	  logerror("**Failed** test #25 (unary operators)\n");
	}
	test_failed = TRUE;
	logerror("    test1_25_globalVariable2 = %p, not %p\n",
	    test1_25_globalVariable2, (void *) &test1_25_globalVariable1);
    }

    if (test1_25_globalVariable3 != test1_25_globalVariable1) {
	if (!test_failed) {
	  logerror("**Failed** test #25 (unary operators)\n");
	}
	test_failed = TRUE;
	logerror("    test1_25_globalVariable3 = %d, not %d\n",
	    test1_25_globalVariable3, test1_25_globalVariable1);
    }

    if (test1_25_globalVariable5 != -test1_25_globalVariable4) {
	if (!test_failed) {
	  logerror("**Failed** test #25 (unary operators)\n");
	}
	test_failed = TRUE;
	logerror("    test1_25_globalVariable5 = %d, not %d\n",
	    test1_25_globalVariable5, -test1_25_globalVariable4);
    }

    if (test1_25_globalVariable7 != -test1_25_globalVariable6) {
	if (!test_failed) {
	  logerror("**Failed** test #25 (unary operators)\n");
	}
	test_failed = TRUE;
	logerror("    test1_25_globalVariable7 = %d, not %d\n",
	    test1_25_globalVariable7, -test1_25_globalVariable6);
    }

    if (!test_failed) {
      logerror("Passed test #25 (unary operators)\n");
      test_passes(testname);
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }
    return retval;
}

void test1_25_call1() {
  DUMMY_FN_BODY;
}
