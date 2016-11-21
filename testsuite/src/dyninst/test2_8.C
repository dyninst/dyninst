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

// $Id: test2_8.C,v 1.1 2008/10/30 19:20:29 legendre Exp $
/*
 * #Name: test2_8
 * #Desc: BPatch_breakPointExpr
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test2_8_Mutator : public DyninstMutator {
  BPatch *bpatch;

  int test8a();
  int test8b();

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_8_factory() {
  return new test2_8_Mutator();
}


// Start of test #8 - BPatch_breakPointExpr
//
//   There are two parts to the mutator side of this test.  The first part
//     (test8a) inserts a BPatch_breakPointExpr into the entry point of
//     the function test8_1.  The secon pat (test8b) waits for this breakpoint
//     to be reached.  The first part is run before the processes is continued
//     (i.e. just after process creation or attach).
//
int test2_8_Mutator::test8a() {
    /*
     * Instrument a function with a BPatch_breakPointExpr.
     */

  BPatch_Vector<BPatch_function *> found_funcs;
  const char *funcname = "test2_8_mutatee";
    if ((NULL == appImage->findFunction(funcname, found_funcs, 1)) || !found_funcs.size()) {
      logerror("    Unable to find function %s\n", funcname);
      return -1;
    }

    if (1 < found_funcs.size()) {
      logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), funcname);
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_entry);

    if (points == NULL) {
	logerror("**Failed** test #8 (BPatch_breakPointExpr)\n");
	logerror("    unable to locate function \"%s\".\n", funcname);
        return -1;
    }

    BPatch_breakPointExpr bp;

    if (appProc->insertSnippet(bp, *points) == NULL) {
	logerror("**Failed** test #8 (BPatch_breakPointExpr)\n");
	logerror("    unable to insert breakpoint snippet\n");
        return -1;
    }

    return 0;
}

int test2_8_Mutator::test8b()
{
  // Wait for process to finish
    if (0 == waitUntilStopped(bpatch, appProc, 8, "BPatch_breakPointExpr")) {
    // waitUntilStopped finished successfully
    logerror("Passed test #8 (BPatch_breakPointExpr)\n");
    return 0;
  } else {
    // waitUntilStopped returned an error code, and already printed out an
    // error message for us
    return -1;
  }
}

test_results_t test2_8_Mutator::executeTest() {
   // Insert a breakpoint into the mutatee
   bpatch = BPatch::getBPatch();

   if ( test8a() < 0 ) {
     return FAILED;
   }
   // Let the mutatee run until it hits the breakpoint.
   appProc->continueExecution();

   if (test8b() < 0) {
     // Something went wrong in test8b.  Probably we got the wrong signal code
     // from the mutatee
     return FAILED;
   } 
   // TODO set the variable test2_8_passed in the mutatee
   BPatch_variableExpr *passed_expr =
      appImage->findVariable("test2_8_passed");
   if (passed_expr == NULL) {
      logerror("**Failed** test #8 (BPatch_breakPointExpr)\n");
      logerror("    Unable to locate test2_8_passed\n");
      return FAILED;
   } else {
      int pvalue = 1;
      passed_expr->writeValue(&pvalue);
      // Continue the mutatee from the breakpoint so it can exit
      appProc->continueExecution();
      return PASSED;
   }
}

// extern "C" TEST_DLL_EXPORT int test2_8_mutatorMAIN(ParameterDict &param)
test_results_t test2_8_Mutator::setup(ParameterDict &param) {
   bool useAttach = ((create_mode_t) param["createmode"]->getInt()) == USEATTACH;
   bpatch = (BPatch *)(param["bpatch"]->getPtr());

    appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    appProc = appThread->getProcess();
    
    // Read the program's image and get an associated image object
    appImage = appProc->getImage();
    if ( useAttach ) {
	  if ( ! signalAttached(appImage) ) {
		  return FAILED;
	  }
  }

    return PASSED;
}
