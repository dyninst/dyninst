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

// $Id: test2_14.C,v 1.1 2007/09/24 16:39:19 cooksey Exp $
/*
 * #Name: test2_14
 * #Desc: Delete Thread
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
class test2_14_Mutator : public TestMutator {
  BPatch *bpatch;

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test2_14_factory() {
  return new test2_14_Mutator();
}

//
// Test 14 - Look through the thread list and make sure the thread has
//    been deleted as required.

// static int mutatorTest(BPatch_thread *thread, BPatch_image *appImage)
test_results_t test2_14_Mutator::execute() {
    killMutatee(appThread); // FIXME Uses deprecated BPatch_thread::terminateExecution()
    delete appThread;
   
    bool failed_this = false;
    BPatch_Vector<BPatch_thread *> *threads = bpatch->getThreads();
    for (unsigned int i=0; i < threads->size(); i++) {
	if ((*threads)[i] == appThread) {
	    logerror("**Failed** test #14 (delete thread)\n"); 
	    logerror("    thread %d was deleted, but getThreads found it\n",
		appThread->getPid());
	    failed_this = true;
	}
    }

    if (!failed_this) {
	logerror("Passed test #14 (delete thread)\n");
        return PASSED;
    } else {
        return FAILED;
    }
}

// extern "C" TEST_DLL_EXPORT int test2_14_mutatorMAIN(ParameterDict &param)
test_results_t test2_14_Mutator::setup(ParameterDict &param) {
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
