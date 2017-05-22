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

int test1_20_func2(int *int_val, double *double_val);
void test1_20_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static int eq_doubles(double a, double b);

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

#define TEST20_A 3
#define TEST20_B 4.3
#define TEST20_C 7
#define TEST20_D 6.4
#define TEST20_TIMES 41

static volatile int ta = TEST20_A;
static volatile double tb = TEST20_B;
static volatile int tc = TEST20_C;
static volatile double td = TEST20_D;
static int test20_iter = 50;

#define TEST20_ANSWER 1088896211

static int globalVariable20_1 = (int)0xdeadbeef;
static double globalVariable20_2 = 0.0;

/* Function definitions follow */

/*
 * Test #20 - instrumentation at arbitrary points
 */

int test1_20_mutatee() 
{
  int retval;
  int ret = 0;
  int int_val = 0;
  double double_val = 0.0;

  ret = test1_20_func2(&int_val, &double_val);

  if (globalVariable20_1 == (TEST20_A * TEST20_TIMES) &&
      eq_doubles(globalVariable20_2, (TEST20_B * (double)TEST20_TIMES)) &&
      int_val == (TEST20_C * TEST20_TIMES) &&
      eq_doubles(double_val, (TEST20_D * (double)TEST20_TIMES)) &&
      ret == TEST20_ANSWER) {
    logerror("Passed test #20 (instrument arbitrary points)\n");
    test_passes(testname);
    retval = 0; /* Test passed */
  } 
  else 
  {
    logerror("**Failed test #20 (instrument arbitrary points)\n");
    if (globalVariable20_1 != (TEST20_A * TEST20_TIMES))
      logerror("    globalVariable20_1 contained %d, not %d as expected\n",
	       globalVariable20_1, TEST20_A * TEST20_TIMES);
    if (!eq_doubles(globalVariable20_2, (TEST20_B * (double)TEST20_TIMES)))
      logerror("    globalVariable20_2 contained %g, not %g as expected\n",
	       globalVariable20_2, TEST20_B * (double)TEST20_TIMES);
    if (int_val != (TEST20_C * TEST20_TIMES))
      logerror("    int_val contained %d, not %d as expected\n",
	       int_val, TEST20_C * TEST20_TIMES);
    if (!eq_doubles(double_val, (TEST20_D * (double)TEST20_TIMES)))
      logerror("    double_val contained %g, not %g as expected\n",
	       double_val, TEST20_D * (double)TEST20_TIMES);
    if (ret != TEST20_ANSWER)
      logerror("    ret contained %d, not %d as expected\n",
	       ret, TEST20_ANSWER);
    retval = -1; /* Test failed */
  }
  return retval;
}

int test1_20_func2(int *int_val, double *double_val) {
    int i, ret = 1;

    *int_val = tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
	       (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
	       (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
	       (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc
	       ))))))))))))))))))))))))))))))))))))))));
    
    *double_val = td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
		  (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
		  (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
		  (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+(td
		  ))))))))))))))))))))))))))))))))))))))));
    for (i = 0; i < test20_iter; i++) {
	ret *= 3;
	if (i % 2 == 1) {
	    ret *= 5;
	} else if (i < 10) {
	    ret *= 7;
	} else if (i > 20) {
	    ret *= 11;
	}
    }
    return ret;
}

/* This function appears to be unused */
int func20_3()
{
    static int n = 1;

    return n++;
}

void test1_20_call1() 
{
	if (debugPrint)
		fprintf(stderr, "%s[%d]:  welcome to test1_20_call1\n", __FILE__, __LINE__);
    globalVariable20_1 = ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+
			 (ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+
			 (ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+
			 (ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta
			 ))))))))))))))))))))))))))))))))))))))));

    globalVariable20_2 = tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+
			 (tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+
			 (tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+
			 (tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb
			 ))))))))))))))))))))))))))))))))))))))));
}

/*
 * Determine if two doubles are close to being equal (for our purposes, that
 * means to ten decimal places).
 */
int eq_doubles(double a, double b) {
    double diff = a - b;

    if (diff < 0) diff = -diff;

    if (diff < 0.00000000001) return 1;
    else return 0;
}
