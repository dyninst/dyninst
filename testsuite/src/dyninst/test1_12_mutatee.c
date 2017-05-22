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

void test1_12_func2();
void test1_12_call1();

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

static volatile int globalVariable12_1 = 0;

/* Function definitions follow */

/*
 * Test #12 - insert/remove and malloc/free
 */	

int test1_12_mutatee() {
  int retval;
  test1_12_func2();
  stop_process_();
  test1_12_func2();
  if (globalVariable12_1 == 1) {
    logerror("Passed test #12 (insert/remove and malloc/free)\n");
    test_passes(testname);
    retval = 0; /* Test passed */
  } else {
    logerror("**Failed test #12 (insert/remove and malloc/free)\n");
    logerror("ZANDY: #12 failed because globalVariable12_1 == %d\n",
	     globalVariable12_1);
    retval = -1;
  }
  return retval;
}

void test1_12_func2() {
  DUMMY_FN_BODY;
}

void test1_12_call1() {
  globalVariable12_1++;
}
