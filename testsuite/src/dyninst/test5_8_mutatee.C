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
decl_test test5_8_test8;
static int inst_called = false;

/* Function definitions follow */

void decl_test::func_cpp()
{
   int CPP_DEFLT_ARG = 8;

   if ( 8 != CPP_DEFLT_ARG )
     logerror("CPP_DEFLT_ARG init value wrong\n");
   if ( 1024 != ::CPP_DEFLT_ARG )
     logerror("::CPP_DEFLT_ARG init value wrong\n");
   if ( 0 != cpp_test_util::CPP_TEST_UTIL_VAR )
       logerror("cpp_test_util::CPP_TEST_UTIL_VAR int value wrong, expected %d, got %d\n", 0, cpp_test_util::CPP_TEST_UTIL_VAR);
}

// A call to this is inserted by the mutator when its analysis succeeds (test
// passes)
void decl_test::call_cpp(int test)
{
  inst_called = true;
   if (test != 8) {
     logerror("**Failed** test #8 (C++ argument pass)\n");
     logerror("    Pass in an incorrect parameter value\n");
     return;
   }
   // Why do we do this assignment?
//    cpp_test_util::CPP_TEST_UTIL_VAR = ::CPP_DEFLT_ARG;
//    cpp_test_util::call_cpp(test);
   passed = 1;
}

int test5_8_mutatee() {
  test5_8_test8.func_cpp();
  if (1 == passed) {
    // Test passed
    logstatus("Passed test #8 (declaration)\n");
    test_passes(testname);
    return 0;
  } else {
    if (!inst_called) {
      logerror("**Failed test #8 (declaration)\n");
      logerror("    Instrumentation not called\n");
    }
    // Test failed
    return -1;
  }
}
