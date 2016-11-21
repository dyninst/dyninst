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

void test1_7_func1();
void test1_7_func2();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_7_globalVariable1 = 71, test1_7_globalVariable2 = 71,
    test1_7_globalVariable3 = 71, test1_7_globalVariable4 = 71,
    test1_7_globalVariable5 = 71, test1_7_globalVariable6 = 71,
    test1_7_globalVariable7 = 71, test1_7_globalVariable8 = 71,
    test1_7_globalVariable9 = 71, test1_7_globalVariable10 = 71,
    test1_7_globalVariable11 = 71, test1_7_globalVariable12 = 71,
    test1_7_globalVariable13 = 71, test1_7_globalVariable14 = 71,
    test1_7_globalVariable15 = 71, test1_7_globalVariable16 = 71;

int test1_7_globalVariable1a = 73, test1_7_globalVariable2a = 73,
    test1_7_globalVariable3a = 73, test1_7_globalVariable4a = 73,
    test1_7_globalVariable5a = 73, test1_7_globalVariable6a = 73,
    test1_7_globalVariable7a = 73, test1_7_globalVariable8a = 73,
    test1_7_globalVariable9a = 73, test1_7_globalVariable10a = 73,
    test1_7_globalVariable11a = 73, test1_7_globalVariable12a = 73,
    test1_7_globalVariable13a = 73, test1_7_globalVariable14a = 73,
    test1_7_globalVariable15a = 73, test1_7_globalVariable16a = 73;

int test1_7_constVar0 = 0;
int test1_7_constVar1 = 1;
int test1_7_constVar2 = 2;
int test1_7_constVar3 = 3;
int test1_7_constVar4 = 4;
int test1_7_constVar5 = 5;
int test1_7_constVar6 = 6;
int test1_7_constVar7 = 7;
int test1_7_constVar9 = 9;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void fail7Print(int tCase, int fCase, const char *op);
static void fail7aPrint(int tCase, int fCase, const char *op);

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int passed = 0;

/* Function definitions follow */

/*
 * Start Test Case #7 - relational operators
 *	Generate all relational operators (eq, gt, le, ne, ge, and, or)
 *	in both the true and false forms.
 */

int test1_7_mutatee() {
  test1_7_func1();
  if (passed) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}

void test1_7_func1() {
  test1_7_func2();
  if ((test1_7_globalVariable1 == 72) && (test1_7_globalVariable2 == 71) &&
      (test1_7_globalVariable3 == 72) && (test1_7_globalVariable4 == 71) &&
      (test1_7_globalVariable5 == 72) && (test1_7_globalVariable6 == 71) &&
      (test1_7_globalVariable7 == 72) && (test1_7_globalVariable8 == 71) &&
      (test1_7_globalVariable9 == 72) && (test1_7_globalVariable10 == 71) &&
      (test1_7_globalVariable11 == 72) && (test1_7_globalVariable12 == 71) &&
      (test1_7_globalVariable13 == 72) && (test1_7_globalVariable14 == 71) &&
      (test1_7_globalVariable15 == 72) && (test1_7_globalVariable16 == 71) &&
      (test1_7_globalVariable1a == 74) && (test1_7_globalVariable2a == 73) &&
      (test1_7_globalVariable3a == 74) && (test1_7_globalVariable4a == 73) &&
      (test1_7_globalVariable5a == 74) && (test1_7_globalVariable6a == 73) &&
      (test1_7_globalVariable7a == 74) && (test1_7_globalVariable8a == 73) &&
      (test1_7_globalVariable9a == 74) && (test1_7_globalVariable10a == 73) &&
      (test1_7_globalVariable11a == 74) && (test1_7_globalVariable12a == 73) &&
      (test1_7_globalVariable13a == 74) && (test1_7_globalVariable14a == 73) &&
      (test1_7_globalVariable15a == 74) && (test1_7_globalVariable16a == 73)) {
    logerror("Passed test #7 (relational operators)\n");
    passed = 1;
  } else {
    logerror("**Failed** test #7 (relational operators)\n");
    fail7Print(test1_7_globalVariable1, test1_7_globalVariable2, "BPatch_lt");
    fail7Print(test1_7_globalVariable3, test1_7_globalVariable4, "BPatch_eq");
    fail7Print(test1_7_globalVariable5, test1_7_globalVariable6, "BPatch_gt");
    fail7Print(test1_7_globalVariable7, test1_7_globalVariable8, "BPatch_le");
    fail7Print(test1_7_globalVariable9, test1_7_globalVariable10, "BPatch_ne");
    fail7Print(test1_7_globalVariable11, test1_7_globalVariable12, "BPatch_ge");
    fail7Print(test1_7_globalVariable13, test1_7_globalVariable14, "BPatch_and");
    fail7Print(test1_7_globalVariable15, test1_7_globalVariable16, "BPatch_or");

    fail7aPrint(test1_7_globalVariable1a, test1_7_globalVariable2a, "BPatch_lt");
    fail7aPrint(test1_7_globalVariable3a, test1_7_globalVariable4a, "BPatch_eq");
    fail7aPrint(test1_7_globalVariable5a, test1_7_globalVariable6a, "BPatch_gt");
    fail7aPrint(test1_7_globalVariable7a, test1_7_globalVariable8a, "BPatch_le");
    fail7aPrint(test1_7_globalVariable9a, test1_7_globalVariable10a, "BPatch_ne");
    fail7aPrint(test1_7_globalVariable11a, test1_7_globalVariable12a, "BPatch_ge");
    fail7aPrint(test1_7_globalVariable13a, test1_7_globalVariable14a, "BPatch_and");
    fail7aPrint(test1_7_globalVariable15a, test1_7_globalVariable16a, "BPatch_or");
    passed = 0;
  }
}

void fail7Print(int tCase, int fCase, const char *op)
{
    if (tCase != 72)
	logerror(" operator %s was not true when it should be - const expr\n",
	    op);
    if (fCase != 71)
       logerror(" operator %s was not false when it should be - const expr\n",
	    op);
}

void fail7aPrint(int tCase, int fCase, const char *op)
{
    if (tCase != 74)
	logerror(" operator %s was not true when it should be - var expr\n", op);
    if (fCase != 73)
	logerror(" operator %s was not false when it should be - var expr\n",op);
}

void test1_7_func2() {
  dprintf("test1_7_func2 () called\n");
}
