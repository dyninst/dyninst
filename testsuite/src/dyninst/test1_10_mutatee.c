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

void test1_10_func1();
void test1_10_call1();
void test1_10_call2();
void test1_10_call3();

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

static int globalVariable10_1 = 0;
static int globalVariable10_2 = 0;
static int globalVariable10_3 = 0;
static int globalVariable10_4 = 0;

static int passed = 0;

/* Function definitions follow */

/*
 * Test #10 - insert snippet order
 *	Verify that a snippet are inserted in the requested order.  We insert
 *	one snippet and then request two more to be inserted.  One before
 *	the first snippet and one after it.
 */
int test1_10_mutatee() {
  test1_10_func1();

  if (passed) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}

void test1_10_func1() {
  if ((globalVariable10_1 == 1) && (globalVariable10_2 == 1) &&
      (globalVariable10_3 == 1) && (globalVariable10_4 == 3)) {
    logerror("Passed test #10 (insert snippet order)\n");
    passed = 1;
  } else {
    logerror("** Failed test #10 (insert snippet order)\n");
    if (!globalVariable10_1)
      logerror("    test1_10_call1 was not called first\n");
    if (!globalVariable10_2)
      logerror("    test1_10_call2 was not called second\n");
    if (!globalVariable10_3)
      logerror("    test1_10_call3 was not called third\n");
    passed = 0;
  }
}

void test1_10_call1() {
  if (globalVariable10_4 == 0) {
    globalVariable10_4 = 1;
    globalVariable10_1 = 1;
  }
}


void test1_10_call2() {
  if (globalVariable10_4 == 1) {
    globalVariable10_4 = 2;
    globalVariable10_2 = 1;
  }
}

void test1_10_call3() {
  if (globalVariable10_4 == 2) {
    globalVariable10_4 = 3;
    globalVariable10_3 = 1;
  }
}
