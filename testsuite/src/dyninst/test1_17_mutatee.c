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
#include <assert.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test1_17_func1();
int test1_17_func2();
int test1_17_call1(int p1);
int test1_17_call2(int p1);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func17_3();
static int func17_4();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalVariable17_1 = 0;
static int globalVariable17_2 = 0;

#define RAN17_1 1701000

#define RET17_1 1700100
#define RET17_2 1700200

/* Function definitions follow */

/*
 * Test #17 - return values from func calls
 *	See test1.C for a detailed comment
 */
int test1_17_func1() {
    int retval;
    int ret17_1;

    ret17_1 = test1_17_func2();
    func17_3();

    if ((ret17_1 != RET17_1) ||
	(globalVariable17_1 != RET17_2) ||
	(globalVariable17_2 != RAN17_1)) {
        logerror("**Failed** test case #17 (return values from func calls)\n");
        if (ret17_1 != RET17_1) {
            logerror("  return value was %d, not %d\n", ret17_1, RET17_1);
        }
        if (globalVariable17_1 != RET17_2) {
            logerror("  return value was %d, not %d\n",
                globalVariable17_1, RET17_2);
        }
        if (globalVariable17_2 != RAN17_1) {
            logerror("  function test1_17_call2 was not inserted\n");
        }
	retval = -1; /* Test failed */
    } else {
        logerror("Passed test #17 (return values from func calls)\n");
	retval = 0; /* Test passed */
    }
    return retval;
}

int test1_17_func2() {
    return RET17_1;
}

void func17_3() {
    globalVariable17_1 = func17_4();
    return;
}

int func17_4() {
    return RET17_2;
}

int test1_17_call1(int p1) {
     /* make sure the function uses lots of registers */

     int a1, a2, a3, a4, a5, a6, a7;

     dprintf("test1_17_call1 (p1=%d)\n", p1);
     assert(p1!=0); /* shouldn't try to divide by zero! */
     assert(p1==1); /* actually only expect calls with p1==1 */

     a1 = p1;
     a2 = a1 + p1;
     a3 = a1 * a2;
     a4 = a3 / p1;
     a5 = a4 + p1;
     a6 = a5 + a1;
     a7 = a6 + p1;

     dprintf("test1_17_call1 (ret=%d)\n", a7);

     return a7;
}

int test1_17_call2(int p1) {
     /* make sure the function uses lots of registers */

     int a1, a2, a3, a4, a5, a6, a7;

     dprintf("test1_17_call2 (p1=%d)\n", p1);
     assert(p1!=0); /* shouldn't try to divide by zero! */
     assert(p1==1); /* actually only expect calls with p1==1 */

     a1 = p1;
     a2 = a1 + p1;
     a3 = a1 * a2;
     a4 = a3 / p1;
     a5 = a4 + p1;
     a6 = a5 + a1;
     a7 = a6 + p1;
     globalVariable17_2 = RAN17_1;

     dprintf("test1_17_call2 (ret=%d)\n", a7);

     return a7;
}

int test1_17_mutatee() {
  if (test1_17_func1() == 0) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}
