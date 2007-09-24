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

// $Id: test_thread_1.C,v 1.1 2007/09/24 16:41:32 cooksey Exp $
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

#include "TestMutator.h"
class test_thread_1_Mutator : public TestMutator {
public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test_thread_1_factory() {
  return new test_thread_1_Mutator();
}

#define TESTNO 1
#define TESTNAME "test_thread_1"
#define TESTDESC "rtlib spinlocks"

static int mutateeXLC;
extern BPatch *bpatch;
static int debugPrint;

static BPatch_thread *appThread;
static const char *expected_fnames[] = {"call1_1","call1_2","call1_3","call1_4"};
static int test1done = 0;
static int test1err = 0;
template class BPatch_Vector<void *>;
static BPatch_Vector<void *> test1handles;
static BPatch_Vector<BPatch_point *> dyncalls;

// static int mutatorTest(BPatch_thread *appT, BPatch_image *appImage)
test_results_t test_thread_1_Mutator::execute() {
  //  Just continue the process and wait for the (mutatee side) test to complete
  BPatch_process *proc = appThread->getProcess();
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
        dprintf("%s[%d]:  BAD NEWS:  process is stopped, something is broken\n",
                __FILE__, __LINE__);
        proc->continueExecution();
     }
     if (proc->isDetached()) {
        dprintf("%s[%d]:  BAD NEWS:  process is detached, something is broken\n",
                __FILE__, __LINE__);
	proc->terminateExecution(); // BUG(?) Will this work?
	return FAILED;
     }
   }
   if (proc->isTerminated()) {
     //fprintf(stderr, "[%s:%u] - mutatee terminated\n", __FILE__, __LINE__); /*DEBUG*/
     switch(proc->terminationStatus()) {
       case ExitedNormally:
         {
          int code = proc->getExitCode();
          dprintf("%s[%d]:  exited normally with code %d\n",
                  __FILE__, __LINE__, code);
          if (code != 0)
	    return FAILED;
          break;
         }
       case ExitedViaSignal:
         {
          int code = proc->getExitSignal();
          dprintf("%s[%d]:  exited with signal %d\n",
                  __FILE__, __LINE__, code);
          return FAILED;
          break;
         }
       case NoExit:
       default:
          dprintf("%s[%d]:  did not exit ???\n",
                  __FILE__, __LINE__);
          return FAILED;
          break;
     };
   }

   if (timeout >= TIMEOUT) {
     FAIL_MES(TESTNAME, TESTDESC);
     dprintf("%s[%d]:  test timed out.\n",
            __FILE__, __LINE__);
     return FAILED;
   }

  PASS_MES(TESTNAME, TESTDESC);
  return PASSED;
}
