/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test2_8.C,v 1.1 2007/09/24 16:39:30 cooksey Exp $
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

#include "test_lib.h"

#include "TestMutator.h"
class test2_8_Mutator : public TestMutator {
  BPatch *bpatch;

  int test8a();
  int test8b();

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test2_8_factory() {
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

    if (appThread->insertSnippet(bp, *points) == NULL) {
	logerror("**Failed** test #8 (BPatch_breakPointExpr)\n");
	logerror("    unable to insert breakpoint snippet\n");
        return -1;
    }

    return 0;
}

int test2_8_Mutator::test8b()
{
  // Wait for process to finish
  if (0 == waitUntilStopped(bpatch, appThread, 8, "BPatch_breakPointExpr")) {
    // waitUntilStopped finished successfully
    logerror("Passed test #8 (BPatch_breakPointExpr)\n");
    return 0;
  } else {
    // waitUntilStopped returned an error code, and already printed out an
    // error message for us
    return -1;
  }
}

test_results_t test2_8_Mutator::execute() {
   // Insert a breakpoint into the mutatee
   if ( test8a() < 0 ) {
     // I need to terminate the mutatee here
     appThread->getProcess()->terminateExecution();
     return FAILED;
   }
   // Let the mutatee run until it hits the breakpoint.
   appThread->continueExecution();

   if (test8b() < 0) {
     // Something went wrong in test8b.  Probably we got the wrong signal code
     // from the mutatee
     if (!appThread->getProcess()->isTerminated()) {
       // Kill the mutatee if it's still running
       appThread->getProcess()->terminateExecution();
     }
     return FAILED;
   } else {
     // TODO set the variable test2_8_passed in the mutatee
     BPatch_variableExpr *passed_expr =
       appImage->findVariable("test2_8_passed");
     if (passed_expr == NULL) {
       logerror("**Failed** test #8 (BPatch_breakPointExpr)\n");
       logerror("    Unable to locate test2_8_passed\n");
       if (!appThread->getProcess()->isTerminated()) {
	 appThread->getProcess()->terminateExecution();
       }
       return FAILED;
     } else {
       int pvalue = 1;
       passed_expr->writeValue(&pvalue);
       // Continue the mutatee from the breakpoint so it can exit
       appThread->continueExecution();
       while (!appThread->getProcess()->isTerminated()) {
	 bpatch->waitForStatusChange();
       }
       return PASSED;
     }
   }
}

// extern "C" TEST_DLL_EXPORT int test2_8_mutatorMAIN(ParameterDict &param)
test_results_t test2_8_Mutator::setup(ParameterDict &param) {
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());

    appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
	signalAttached(appThread, appImage);
    }

    return PASSED;
}
