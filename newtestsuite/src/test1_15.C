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

// $Id: test1_15.C,v 1.1 2007/09/24 16:36:43 cooksey Exp $
/*
 * #Name: test1_15
 * #Desc: Mutator Side - setMutationsActive
 * #Dep: 
 * #Notes: Combined Result of 15a and 15b from previous test, we may want to consider recombining them
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

// static BPatch *bpatch;

#include "TestMutator.h"

class test1_15_Mutator : public TestMutator {
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
  int mutatorTest15a();
  int mutatorTest15b();
};
extern "C" TEST_DLL_EXPORT TestMutator *test1_15_factory() {
  return new test1_15_Mutator();
}

//
// Start Test Case #15 - mutator side (setMutationsActive)
//
int test1_15_Mutator::mutatorTest15a() {
    RETURNONFAIL(insertCallSnippetAt(appThread, appImage, "test1_15_func2",
				     BPatch_entry, "test1_15_call1", 15,
				     "setMutationsActive"));

    /*
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    // On the Sparc, functions containing system calls are relocated into the
    // heap when instrumented, making a special case we should check.

    // "access" makes the "access" system call, so we'll instrument it
    RETURNONFAIL(insertCallSnippetAt(appThread, appImage, "access", BPatch_entry,
	"call15_2", 15, "setMutationsActive"));

    // We want to instrument more than one point, so do exit as well
    RETURNONFAIL(insertCallSnippetAt(appThread, appImage, "access", BPatch_exit,
	"call15_2", 15, "setMutationsActive"));
#endif
    */

    RETURNONFAIL(replaceFunctionCalls(appThread, appImage, "test1_15_func4",
				      "test1_15_func3",	"test1_15_call3", 15,
				      "setMutationsActive", 1));

    return 0;
}

// static int mutatorTest15b(BPatch_thread *appThread, BPatch_image * /*appImage*/)
// {
int test1_15_Mutator::mutatorTest15b() {
    RETURNONFAIL(waitUntilStopped(bpatch, appThread, 15, "setMutationsActive"));

    // disable mutations and continue process
    appThread->setMutationsActive(false);
    appThread->continueExecution();
    
    RETURNONFAIL(waitUntilStopped(bpatch, appThread, 15, "setMutationsActive"));

    // re-enable mutations and continue process
    appThread->setMutationsActive(true);
    appThread->continueExecution();

    return 0;
}

// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_15_Mutator::execute() {
  if (mutatorTest15a() != 0) {
    // Kill mutatee
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }
  while (!appThread->isStopped()) { // Necessary in fast & loose mode
    bpatch->waitForStatusChange();
  }
  appThread->continueExecution();
  if (mutatorTest15b() != 0) {
    // Kill mutatee
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }

  while (!appThread->isTerminated()) {
    bpatch->waitForStatusChange();
  }
  // I need to check the return code of the mutatee also
  if (appThread->getProcess()->terminationStatus() == ExitedNormally) {
    if (appThread->getProcess()->getExitCode() == 0) {
      return PASSED;
    } else {
      return FAILED;
    }
  } else {
    return FAILED;
  }
}

test_results_t test1_15_Mutator::setup(ParameterDict &param) {
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    if ( useAttach )
    {
      if ( ! signalAttached(appThread, appImage) )
         return FAILED;
    }

    return PASSED;
}
