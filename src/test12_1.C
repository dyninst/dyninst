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

// $Id: test12_1.C,v 1.5 2006/06/08 12:25:13 jaw Exp $
/*
 * #Name: test12_1
 * #Desc: rtlib spinlocks
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test12.h"

#define TESTNO 1
#define TESTNAME "rtlib spinlocks"

int mutateeXLC;
extern BPatch *bpatch;
int debugPrint;

BPatch_thread *appThread;
static const char *expected_fnames[] = {"call1_1","call1_2","call1_3","call1_4"};
int test1done = 0;
int test1err = 0;
template class BPatch_Vector<void *>;
BPatch_Vector<void *> test1handles;
BPatch_Vector<BPatch_point *> dyncalls;

int mutatorTest(BPatch_thread *appT, BPatch_image *appImage)
{
  //  Just continue the process and wait for the (mutatee side) test to complete
  BPatch_process *proc = appT->getProcess();
  int childpid = proc->getPid();
  dprintf("%s[%d]:  mutatee process: %d\n", __FILE__, __LINE__, childpid);

  proc->continueExecution();

   int timeout = 0;
   while (timeout < TIMEOUT) {
     sleep_ms(SLEEP_INTERVAL/*ms*/);
     timeout += SLEEP_INTERVAL;
     if (proc->isTerminated())
        break;
     if (proc->isStopped()) {
        //  This really shouldn't happen
        fprintf(stderr, "%s[%d]:  BAD NEWS:  process is stopped, something is broken\n",
                __FILE__, __LINE__);
        proc->continueExecution();
     }
     if (proc->isDetached()) {
        fprintf(stderr, "%s[%d]:  BAD NEWS:  process is detached, something is broken\n",
                __FILE__, __LINE__);
        abort();
     }
   }
   if (proc->isTerminated()) {
     switch(proc->terminationStatus()) {
       case ExitedNormally:
         {
          int code = proc->getExitCode();
          dprintf("%s[%d]:  exited normally with code %d\n",
                  __FILE__, __LINE__, code);
          if (code != 0) return 0;
          break;
         }
       case ExitedViaSignal:
         {
          int code = proc->getExitSignal();
          fprintf(stderr, "%s[%d]:  exited with signal %d\n",
                  __FILE__, __LINE__, code);
          return 0;
          break;
         }
       case NoExit:
       default:
          fprintf(stderr, "%s[%d]:  did not exit ???\n",
                  __FILE__, __LINE__);
          return 0;
          break;
     };
   }

   if (timeout >= TIMEOUT) {
     FAIL_MES(TESTNO, TESTNAME);
     fprintf(stderr, "%s[%d]:  test timed out.\n",
            __FILE__, __LINE__);
     return 0;
   }

  PASS_MES(TESTNO,TESTNAME);
  delete(proc);
  return 1;
}


extern "C" int mutatorMAIN(ParameterDict &param)
{
   fprintf(stderr, "%s[%d]:  welcome to mutatorMAIN for test12_1\n", __FILE__, __LINE__);
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();
    mutateeXLC = param["mutateeXLC"]->getInt();

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
        signalAttached(appThread, appImage);
    }

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
