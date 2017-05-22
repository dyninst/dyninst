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

// $Id: test2_5.C,v 1.1 2008/10/30 19:20:23 legendre Exp $
/*
 * #Name: test2_5
 * #Desc: Look up nonexistent function
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

#include "dyninst_comp.h"
class test2_5_Mutator : public DyninstMutator {
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_5_factory() {
  return new test2_5_Mutator();
}

//
// Test #5 - look up nonexistent function)
//	Try to call findFunction on a function that is not defined for the
//	process.
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *img)
test_results_t test2_5_Mutator::executeTest()
{
    test_results_t result;
    clearError();
    /*
    expectErrors = true; // test #5 causes error #100 (Unable to find function)
    expectErrors = false;
    */
    setExpectError(100); // test #5 causes error #100 (Unable to find function)

    BPatch_Vector<BPatch_function *> bpfv, *res;
    const char *fn = "NoSuchFunction";
    // logerror("Looking for function\n");
    if (!(NULL == (res=appImage->findFunction(fn, bpfv)) || !bpfv.size()
	|| NULL == bpfv[0]) || !getError()){
      logerror("**Failed** test #5 (look up nonexistent function)\n");
      if (res)
	logerror("    non-null for findFunction on non-existant func\n");
      if (!getError())
	logerror("    the error callback should have been called but wasn't\n");
      result = FAILED;
    } else {
	logerror("Passed test #5 (look up nonexistent function)\n");
        result = PASSED;
    }

    setExpectError(DYNINST_NO_ERROR);

    return result;
}
