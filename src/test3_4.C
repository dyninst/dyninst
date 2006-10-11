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

// $Id: test3_4.C,v 1.5 2006/10/11 21:53:36 cooksey Exp $
/*
 * #Name: test3_4
 * #Desc: sequential multiple-process management - exit
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

static const unsigned int MAX_MUTATEES = 32;
static unsigned int Mutatees=3;
static int debugPrint;

//
// Start Test Case #4 - create one process, wait for it to exit.  Then 
//     create a second one and wait for it to exit.  Repeat as required.
//
static int mutatorTest(char *pathname, BPatch *bpatch)
{
    unsigned int n=0;
    const char *child_argv[5];
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");
    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("2");     	// run test2 in mutatee
    child_argv[n++] = NULL;

    BPatch_thread *appThread;

    for (n=0; n<Mutatees; n++) {
        // Start the mutatee
        dprintf("Starting \"%s\" %d/%d\n", pathname, n, Mutatees);
        appThread = bpatch->createProcess(pathname, child_argv, NULL);
        if (!appThread) {
            logerror("*ERROR*: unable to create handle%d for executable\n", n);
            logerror("**Failed** Test #4 (sequential multiple-process management - exit)\n");
            return -1;
        }
        dprintf("Mutatee %d started, pid=%d\n", n, appThread->getPid());

        appThread->continueExecution();

        while (!appThread->isTerminated()) {
           if (appThread->isStopped())
              appThread->continueExecution();
           bpatch->waitForStatusChange();
        }

        if(appThread->terminationStatus() == ExitedNormally) {
           int exitCode = appThread->getExitCode();
           if (exitCode || debugPrint)
               dprintf("Mutatee %d exited with exit code 0x%x\n", n, exitCode);
        } else if(appThread->terminationStatus() == ExitedViaSignal) {
           int signalNum = appThread->getExitSignal();
           if (signalNum || debugPrint)
               dprintf("Mutatee %d exited from signal 0x%d\n", n, signalNum);
        }
	delete appThread;
    }

    logerror("Passed Test #4 (sequential multiple-process management - exit)\n");
    return 0;
}

extern "C" TEST_DLL_EXPORT int test3_4_mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    char *pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    // Get log file pointers
    FILE *outlog = (FILE *)(param["outlog"]->getPtr());
    FILE *errlog = (FILE *)(param["errlog"]->getPtr());
    setOutputLog(outlog);
    setErrorLog(errlog);

#if defined (sparc_sun_solaris2_4)
    // we use some unsafe type operations in the test cases.
    bpatch->setTypeChecking(false);
#endif
    
    return mutatorTest(pathname, bpatch);
}
