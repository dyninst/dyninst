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

#define MAX_TEST_CPP 12
int passedTestCPP[MAX_TEST_CPP+1];

int CPP_DEFLT_ARG = CPP_DEFLT_ARG_VAL;

/*
 * The test functions.
 */

void cpp_test_util::call_cpp(int test)
{
  // Illegally called by test5_9 to trigger inclusion of this
  // function in the dwarf. 
  if (test > MAX_TEST_CPP) return;

   passedTestCPP[test] = TRUE;

   switch (test) {

    case  1 : {
      logerror("Passed test #1 (C++ argument pass)\n");
      break;
    }

    case  2 : {
      logerror("Passed test #2 (overload function)\n");
      break;
    }
    
    case  3 : {
      logerror("Passed test #3 (overload operator)\n");
      break;
    }

    case  4 : {
      logerror("Passed test #4 (static member)\n");
      break;
    }
    
    case  5 : {
      logerror("Passed test #5 (namespace)\n");
      break;
    }

    case  7 : {
      logerror("Passed test #7 (template)\n");
      break;
    }

    case  8 : {
      logerror("Passed test #8 (declaration)\n");
      break;
    }

    case  9 : {
      logerror("Passed test #9 (derivation)\n");
      break;
    }

    case  12 : {
      logerror("Passed test #12 (C++ member function)\n");
      break;
    }

    default : {
      logerror("\tInvalid test %d requested\n", test);
      logerror("\tThis invalid test# is most likely caused by the C++ class member function argument passing\n");
      break;
    }

  }
}
