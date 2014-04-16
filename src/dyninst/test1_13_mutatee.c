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

int test1_13_func1(int p1, int p2, int p3, int p4, int p5);
int test1_13_func2();
void test1_13_func3();

void test1_13_call1(int a1, int a2, int a3, int a4, int a5);
void test1_13_call2(int ret);

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

static int globalVariable13_1 = 0;
static int called_func_3 = 0;

#define RET13_1 1300100

/* Function definitions follow */

/*
 * Test #13 - paramExpr,retExpr,nullExpr
 *	Test various expressions
 */

int test1_13_mutatee() {
  int retval;

  retval = test1_13_func1(131, 132, 133, 134, 135);
  
  if (0 == retval && called_func_3) {
    test_passes(testname);
  }
  return retval;
}

int test1_13_func2()
{
    return(RET13_1);
}
void test1_13_func3()
{
  called_func_3 = 1;
}

int test1_13_func1(int p1, int p2, int p3, int p4, int p5)
{
  int retval;
  if(test1_13_func2() != RET13_1) 
  {
    logerror("Failed test #13; instrumentation affected return value of func2\n");
    return -1;
  }
    if ((p1 == 131) && (p2 == 132) && (p3 == 133) &&
	(p4 == 134) && (p5 == 135) && (globalVariable13_1 == 63)) {
	logerror("Passed test #13 (paramExpr,retExpr,nullExpr)\n");
	retval = 0; /* Test passed */
    } else {
	logerror("**Failed test #13 (paramExpr,retExpr,nullExpr)\n");
	if (p1 != 131) logerror("  parameter 1 is %d, not 131\n", p1);
	if (p2 != 132) logerror("  parameter 2 is %d, not 132\n", p2);
	if (p3 != 133) logerror("  parameter 3 is %d, not 133\n", p3);
	if (p4 != 134) logerror("  parameter 4 is %d, not 134\n", p4);
	if (p5 != 135) logerror("  parameter 5 is %d, not 135\n", p5);
	if (!(globalVariable13_1 & 1)) logerror("    passed param a1 wrong\n");
	if (!(globalVariable13_1 & 2)) logerror("    passed param a2 wrong\n");
	if (!(globalVariable13_1 & 4)) logerror("    passed param a3 wrong\n");
	if (!(globalVariable13_1 & 8)) logerror("    passed param a4 wrong\n");
	if (!(globalVariable13_1 & 16)) logerror("    passed param a5 wrong\n");
	if (!(globalVariable13_1 & 32)) logerror("    return value wrong\n");
	retval = -1; /* Test failed */
    }
    test1_13_func3();
    return retval;
}

void test1_13_call1(int a1, int a2, int a3, int a4, int a5)
{
    if (a1 == 131) globalVariable13_1 |= 1;
    if (a2 == 132) globalVariable13_1 |= 2;
    if (a3 == 133) globalVariable13_1 |= 4;
    if (a4 == 134) globalVariable13_1 |= 8;
    if (a5 == 135) globalVariable13_1 |= 16;
    dprintf("a1 = %d\n", a1);
    dprintf("a2 = %d\n", a2);
    dprintf("a3 = %d\n", a3);
    dprintf("a4 = %d\n", a4);
    dprintf("a5 = %d\n", a5);
}

void test1_13_call2(int ret)
{
    if (ret == RET13_1) {
		globalVariable13_1 |= 32;
	} else {
		dprintf("expected ret %d, actual %d\n", RET13_1, ret);
	}
}
