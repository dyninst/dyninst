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
#include "cpp_test.h"
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

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

static int passed = 0;
exception_test test5_6_test6;

/* Function definitions follow */

void sample_exception::response()
{
   DUMMY_FN_BODY;
}


void test5_6_passed(int arg)
{
   if ( arg == 6 ) {
     passed = 1;
     logerror("Passed test #6 (exception)\n");
   } else {
     logerror("**Failed** test #6 (exception)\n");
   }
}


void exception_test::call_cpp()
{
/* With our current, "catch-block detection", we cannot find the catch blocks that this test seeks since throw is 
   a non-returning function. This is currently fixed by adding a return path from this function.
   Hence adding a path that will never be taken (volatile a = 0;) */
/* This code should be fixed after fixing bug 1154 */

  volatile int a; 
  a = 0; 
  if(!a) 
  	throw sample_exception();
  else
  	return;
}


void exception_test::func_cpp()
{
  try {
    int testno = 6;
    call_cpp();
  }
  catch ( sample_exception & ex) {
    ex.response();
    return;
  }
  catch ( ... ) {
    logerror("**Failed** test #6 (exception)\n");
    logerror("    Does not catch appropriate exception\n");
    throw;
  }
  logerror("Missed proper exception\n");
}

int test5_6_mutatee() {
  test5_6_test6.func_cpp();
  // FIXME Make sure the error reporting works
  // I need to have this guy call test_passes(testname) if the test passes..
  if (1 == passed) {
    // Test passed
    test_passes(testname);
    return 0;
  } else {
    // Test failed
    return -1;
  }
}
