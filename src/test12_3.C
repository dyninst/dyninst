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

// $Id: test12_3.C,v 1.2 2005/10/17 19:14:29 bpellin Exp $
/*
 * #Name: test12_3
 * #Desc: thread create callback
 * #Dep: 
 * #Arch: all
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test12.h"

#define TESTNO 3
#define TESTNAME "thread create callback"

int debugPrint;
BPatch *bpatch;

int test3_threadCreateCounter = 0;
void threadCreateCB(BPatch_process * /*proc*/, BPatch_thread *thr)
{
  if (debugPrint)
     fprintf(stderr, "%s[%d]:  thread %lu start event for pid %d", __FILE__, __LINE__,
               thr->getTid(), thr->getPid());
  test3_threadCreateCounter++;
//  fprintf(stderr, "%s[%d]:  got a thread start event: %d\n", __FILE__, __LINE__,
//          test3_threadCreateCounter);
}

int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
  unsigned int timeout = 0; // in ms
  int err = 0;

  BPatchAsyncThreadEventCallback createcb = threadCreateCB;
  if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent, createcb)) 
  {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to register thread callback\n",
           __FILE__, __LINE__);
    return -1;
  }
  //  unset mutateeIde to trigger thread (10) spawn.
  int zero = 0;
  setVar(appImage, "mutateeIdle", (void *) &zero, TESTNO, TESTNAME);
  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  while(test3_threadCreateCounter < TEST3_THREADS && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    err = 1;
  }

  appThread->stopExecution();

  if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, createcb)) {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to remove thread callback\n",
           __FILE__, __LINE__);
    return -1;
  }
  if (!err)  {
    PASS_MES(TESTNO, TESTNAME);
    return 0;
  }
  return -1;
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
        signalAttached(appThread, appImage);
    }

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
