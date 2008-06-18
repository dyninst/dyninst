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

// $Id: test3_1.C,v 1.3 2008/06/18 19:58:19 carl Exp $
/*
 * #Name: test3_1
 * #Desc: Create processes, process events, and kill them, no instrumentation
 * #Dep: 
 * #Arch:
 * #Notes:useAttach does not apply
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

#define MAX_MUTATEES	32

#include "TestMutator.h"
class test3_1_Mutator : public TestMutator {
  // These used to be static.  I don't think they need to be any more, now that
  // they're instance fields
  unsigned int Mutatees;
  int debugPrint;

  BPatch_exitType expectedSignal;
  
  BPatch *bpatch;
  char *pathname;
public:
  test3_1_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test3_1_factory() {
  return new test3_1_Mutator();
}

test3_1_Mutator::test3_1_Mutator() {
#if defined(os_windows)
  expectedSignal = ExitedNormally;
#else
  expectedSignal = ExitedViaSignal;
#endif

  Mutatees = 3;

  bpatch = NULL;
  pathname = NULL;
} // test3_1_Mutator()

//
// Start Test Case #1 - create processes and process events from each
//     Just let them run a while, then kill them, no instrumentation added.
//
// static int mutatorTest(char *pathname, BPatch *bpatch)
// {
test_results_t test3_1_Mutator::execute() {
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_1"); // run test1 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread[MAX_MUTATEES];

    for (n = 0; n < MAX_MUTATEES; n++) {
      appThread[n] = NULL;
    }

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            MopUpMutatees(n-1,appThread);
            return FAILED;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread[n]->getPid());
	// Register mutatee for cleanup
	registerPID(appThread[n]->getProcess()->getPid());
    }

    dprintf("Letting mutatee processes run a short while (5s).\n");
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

    P_sleep(5);
    dprintf("Terminating mutatee processes.\n");

    appThread[0]->getProcess(); // ???
    unsigned int numTerminated=0;
    for (n=0; n<Mutatees; n++) {
        bool dead = appThread[n]->terminateExecution();
        if (!dead || !(appThread[n]->isTerminated())) {
            logerror("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            logerror("    mutatee process [%d] was not terminated\n", n);
            continue;
        }
        if(appThread[n]->terminationStatus() != expectedSignal) {
            logerror("**Failed** test #1 (simultaneous multiple-process management - terminate)\n");
            logerror("    mutatee process [%d] didn't get notice of termination\n", n);
            continue;
        }
        int signalNum = appThread[n]->getExitSignal();
        dprintf("Terminated mutatee [%d] from signal 0x%x\n", n, signalNum);
        numTerminated++;
    }

    if (numTerminated == Mutatees) {
	logerror("Passed Test #1 (simultaneous multiple-process management - terminate)\n");
        return PASSED;
    }

    return FAILED;
}

// extern "C" TEST_DLL_EXPORT int test3_1_mutatorMAIN(ParameterDict &param)
// {
test_results_t test3_1_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
//     return mutatorTest(pathname, bpatch);
    return PASSED;
} // test3_1_Mutator::setup(ParameterDict &)
