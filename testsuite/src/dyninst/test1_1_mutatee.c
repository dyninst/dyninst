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
#include <stdlib.h>

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_1_call1_1();
void test1_1_func1_1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func1_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalVariable1_1 = 0;

/* Function definitions follow */

void test1_1_call1_1() {
  dprintf("call1() called - setting globalVariable1_1 = 11\n");
  globalVariable1_1 = 11;
}

static void func1_2() {
  dprintf("func1_2 () called, address is %p\n", &func1_2);

}

void test1_1_func1_1() {
  dprintf("Value of globalVariable1_1 is %d.\n", globalVariable1_1);
  dprintf("Address of func1_2 is %p, calling now\n", &func1_2);
  func1_2();
  dprintf("Value of globalVariable1_1 is now %d.\n", globalVariable1_1);

  if (globalVariable1_1 == 11) {
    logerror("\nPassed test #1 (zero arg function call)\n");
    /* test_passes(testname); */
  } else {
    logerror("\n**Failed** test #1 (zero arg function call)\n");
    logerror("\tglobalVariable1_1 = %d, not 11\n", globalVariable1_1);
  }
  flushOutputLog();
}

int test1_1_mutatee() {
  dprintf("Address of func1_1 is %p, calling func1_1\n", &test1_1_func1_1);
  test1_1_func1_1();
  if (11 == globalVariable1_1) {
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}
