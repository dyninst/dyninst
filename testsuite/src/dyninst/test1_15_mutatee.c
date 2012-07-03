/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_15_func2();
void test1_15_func3();
void test1_15_func4();
void test1_15_call1();
void test1_15_call3();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void check15result(const char *varname, int value, int expected,
			  const char *errstr, int *failed);

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalVariable15_1 = 0;
//static int globalVariable15_2 = 0;
static int globalVariable15_3 = 0;
static int globalVariable15_4 = 0;

/* Function definitions follow */

/*
 * Test #15 - setMutationsActive
 */
void check15result(const char *varname, int value, int expected,
                   const char *errstr, int *failed)
{
    if (value != expected) {
	if (!*failed)
	    logerror("**failed test #15 (setMutationsActive)\n");
	*failed = TRUE;

	logerror("    %s = %d %s\n", varname, value, errstr);
    }		
}


void test1_15_func2()
{
    DUMMY_FN_BODY;
}

void test1_15_func3()
{
    globalVariable15_3 = 100;
    /* increment a dummy variable to keep alpha code generator from assuming
       too many free registers on the call into test1_15_func3. jkh 3/7/00 */
    globalVariable15_4++;
}

void test1_15_func4()
{
    test1_15_func3();
}

int test1_15_mutatee() {
    int failed = FALSE;
    int retval;

    test1_15_func2();
    check15result("globalVariable15_1", globalVariable15_1, 1,
		  "after first call to instrumented function", &failed);

    test1_15_func4();
    check15result("globalVariable15_3", globalVariable15_3, 1,
		  "after first call to instrumented function", &failed);

    /***********************************************************/

    stop_process_();

    test1_15_func2();
    check15result("globalVariable15_1", globalVariable15_1, 1,
		  "after second call to instrumented function", &failed);

    test1_15_func4();
    check15result("globalVariable15_3", globalVariable15_3, 100,
		  "after second call to instrumented function", &failed);

    /***********************************************************/

    stop_process_();

    test1_15_func2();
    check15result("globalVariable15_1", globalVariable15_1, 2,
		  "after third call to instrumented function", &failed);
    test1_15_func4();
    check15result("globalVariable15_3", globalVariable15_3, 101,
		  "after third call to instrumented function", &failed);

    if (!failed) {
        logerror("Passed test #15 (setMutationsActive)\n");
	test_passes("test1_15");
	retval = 0; /* Test passed */
    } else {
        retval = -1; /* Test failed */
    }
    return retval;
}

void test1_15_call1() {
    globalVariable15_1++;
}


void test1_15_call3() {
    globalVariable15_3++;
}
