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

// $Id: test3_2.C,v 1.1 2007/09/24 16:39:37 cooksey Exp $
/*
 * #Name: test3_2
 * #Desc: simultaneous multiple-process management - exit
 * #Dep: 
 * #Arch: all
 * #Notes: useAttach does not apply
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test3.h"

#include "TestMutator.h"
class test3_2_Mutator : public TestMutator {
  const unsigned int MAX_MUTATEES;
  unsigned int Mutatees;
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test3_2_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test3_2_factory() {
  return new test3_2_Mutator();
}

test3_2_Mutator::test3_2_Mutator()
  : MAX_MUTATEES(32), Mutatees(3), bpatch(NULL), pathname(NULL) {
}

//
// Start Test Case #2 - create processes and process events from each
//     Just let them run to finish, no instrumentation added.
//
// static int mutatorTest(char *pathname, BPatch *bpatch)
test_results_t test3_2_Mutator::execute() {
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test3_2"); // run test2 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread[MAX_MUTATEES];

    for (n=0; n<MAX_MUTATEES; n++) appThread[n]=NULL;

    // Start the mutatees
    for (n=0; n<Mutatees; n++) {
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread[n] = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread[n]) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** test #2 (simultaneous multiple-process management - exit)\n");
            MopUpMutatees(n-1,appThread);
            return FAILED;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread[n]->getPid());
    }
    dprintf("Letting %d mutatee processes run.\n", Mutatees);
    for (n=0; n<Mutatees; n++) appThread[n]->continueExecution();

    unsigned int numTerminated=0;
    bool terminated[MAX_MUTATEES];
    for (n=0; n<Mutatees; n++) terminated[n]=false;

    // monitor the mutatee termination reports
    while (numTerminated < Mutatees) {
        bpatch->waitForStatusChange();
        dprintf("%s[%d]:  got status change\n", __FILE__, __LINE__);
        for (n=0; n<Mutatees; n++)
            if (!terminated[n] && (appThread[n]->isTerminated())) {
                if(appThread[n]->terminationStatus() == ExitedNormally) {
                    int exitCode = appThread[n]->getExitCode();
                    if (exitCode || debugPrint)
                        dprintf("Mutatee %d exited with exit code 0x%x\n", n,
                                exitCode);
                }
                else if(appThread[n]->terminationStatus() == ExitedViaSignal) {
                    int signalNum = appThread[n]->getExitSignal();
                    if (signalNum || debugPrint)
                        dprintf("Mutatee %d exited from signal 0x%d\n", n,
                                signalNum);
                }
                terminated[n]=true;
		delete appThread[n];
                numTerminated++;
            }
            else if (!terminated[n] && (appThread[n]->isStopped())) {
                appThread[n]->continueExecution();
            }
    }

    if (numTerminated == Mutatees) {
	logerror("Passed Test #2 (simultaneous multiple-process management - exit)\n");
        return PASSED;
    }
	return FAILED;
}

// extern "C" TEST_DLL_EXPORT int test3_2_mutatorMAIN(ParameterDict &param)
test_results_t test3_2_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
    return PASSED;
}
