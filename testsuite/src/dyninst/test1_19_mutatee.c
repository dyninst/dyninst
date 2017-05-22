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

void test1_19_call1();
void test1_19_call2();

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

static int globalVariable19_1 = (int)0xdeadbeef;
static int globalVariable19_2 = (int)0xdeadbeef;

#define MAGIC19_1 1900100
#define MAGIC19_2 1900200

/* Function definitions follow */

/*
 * Test #19 - oneTimeCode
 */
int test1_19_mutatee() {
    int retval = 0;

    stop_process_();

    if (globalVariable19_1 != MAGIC19_1) {
	logerror("**Failed test #19 (oneTimeCode)\n");
	logerror("    globalVariable19_1 contained %d, not %d as expected\n",
		 globalVariable19_1, MAGIC19_1);
	retval = -1; /* Test failed */
    }

    stop_process_();

    if (globalVariable19_2 == MAGIC19_2) {
        if (0 == retval) {
	  logerror("Passed test #19 (oneTimeCode)\n");
	  test_passes(testname);
	  retval = 0; /* Test passed */
	}
    } else {
	logerror("**Failed test #19 (oneTimeCode)\n");
	logerror("    globalVariable19_2 contained %d, not %d as expected\n",
		 globalVariable19_2, MAGIC19_2);
	retval = -1; /* Test failed */
    }
    return retval;
}

void test1_19_call1() {
    globalVariable19_1 = MAGIC19_1;
}

void test1_19_call2() {
    globalVariable19_2 = MAGIC19_2;
}
