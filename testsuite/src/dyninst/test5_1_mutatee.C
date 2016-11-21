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

/* Function definitions follow */

arg_test test1; 

void arg_test::func_cpp()
{
  int test = 1;
  int arg2 = 1;

  call_cpp(test, arg2);
}

void arg_test::arg_pass(int test)
{
   if (test != 1 || value != 5) {
     logerror("**Failed** test #1 (C++ argument pass)\n");
     logerror("    Passed in an incorrect parameter value:\n");
     logerror("    test = %d, this = %u, test1 = %u\n",
	      test, (void *)this, (void *)&test1);
//      cerr << "    test = " << test << ", this = " << (void *) this;
//       cerr << ", test1 = " << (void *) &test1;
      return;
   }
   logerror("Passed test #1 (C++ argument pass)\n");
   test_passes(testname);
}

void arg_test::dummy()
{
  DUMMY_FN_BODY;
}

void arg_test::call_cpp(const int arg1, int & arg2, int arg3)
{
   const int m = 8;
   int n = 6;
   int & reference = n;
   int dummystringthatisunique = 42;

   dummy(); // place to change the value of arg3 from CPP_DEFLT_ARG to 1 

   if ( 1 != arg3 ) {
     logerror("**Failed** test #1 (C++ argument pass)\n");
     logerror("    Default argument value is not changed\n");
   }

   if ( arg1 == arg2 )  arg2 = CPP_DEFLT_ARG;
}

int test5_1_mutatee() {
  test1.func_cpp();
  // FIXME Make sure the error reporting works
  // I need to have this guy call test_passes(testname) if the test passes..
  return 0; /* No error */
}
