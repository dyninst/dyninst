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

// $Id: test_thread_3.C,v 1.1 2007/09/24 16:41:36 cooksey Exp $
/*
 * #Name: test12_4
 * #Desc: thread create callback -- doa
 * #Dep: 
 * #Arch: all
 * #Notes:
 */

#include <vector>
using std::vector;
#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test12.h"

#include "TestMutator.h"
class test_thread_3_Mutator : public TestMutator {
private:
  BPatch *bpatch;

  void dumpVars();
  bool setVar(const char *vname, void *addr, int testno, const char *testname);
  bool getVar(const char *vname, void *addr, int len, int testno,
	      const char *testname);

public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test_thread_3_factory() {
  return new test_thread_3_Mutator();
}

#define TESTNO 4
#define TESTNAME "test_thread_3"
#define TESTDESC "thread create callback - doa"

static int debugPrint;

void test_thread_3_Mutator::dumpVars() {
  BPatch_Vector<BPatch_variableExpr *> vars;
  appImage->getVariables(vars);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    logerror("\t%s\n", vars[i]->getName());
  }
}

// Returns false on success, true on error
bool test_thread_3_Mutator::setVar(const char *vname, void *addr, int testno,
				  const char *testname) {
   BPatch_variableExpr *v;
   void *buf = addr;
   if (NULL == (v = appImage->findVariable(vname))) {
      logerror("**Failed test #%d (%s)\n", testno, testname);
      logerror("  cannot find variable %s, avail vars:\n", vname);
      dumpVars();
      return true;
   }

   if (! v->writeValue(buf, sizeof(int),true)) {
      logerror("**Failed test #%d (%s)\n", testno, testname);
      logerror("  failed to write call site var to mutatee\n");
      return true;
   }
   return false;
}

// Returns false on success, true on error
bool test_thread_3_Mutator::getVar(const char *vname, void *addr, int len,
				   int testno, const char *testname) {
   BPatch_variableExpr *v;
   if (NULL == (v = appImage->findVariable(vname))) {
      logerror("**Failed test #%d (%s)\n", testno, testname);
      logerror("  cannot find variable %s: avail vars:\n", vname);
      dumpVars();
      return true;
   }

   if (! v->readValue(addr, len)) {
      logerror("**Failed test #%d (%s)\n", testno, testname);
      logerror("  failed to read var in mutatee\n");
      return true;
   }
   return false;
}



static vector<unsigned long> callback_tids;
static int test3_threadCreateCounter = 0;
static void threadCreateCB(BPatch_process * proc, BPatch_thread *thr)
{
  assert(thr);  
  if (debugPrint)
     dprintf("%s[%d]:  thread %lu start event for pid %d\n", __FILE__, __LINE__,
               thr->getTid(), thr->getPid());
  test3_threadCreateCounter++;
  callback_tids.push_back(thr->getTid());
  if (thr->isDeadOnArrival()) {
     dprintf("%s[%d]:  thread %lu is doa \n", __FILE__, __LINE__, thr->getTid());
  }
}

// static bool mutatorTest3and4(int testno, const char *testname)
test_results_t test_thread_3_Mutator::execute() {
  test3_threadCreateCounter = 0;
  callback_tids.clear();

  unsigned int timeout = 0; // in ms
  int err = 0;

  BPatchAsyncThreadEventCallback createcb = threadCreateCB;
  if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent, createcb))
  {
    FAIL_MES(TESTNAME, TESTDESC);
    logerror("%s[%d]:  failed to register thread callback\n",
           __FILE__, __LINE__);
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }

#if 0
  //  unset mutateeIde to trigger thread (10) spawn.
  int zero = 0;
  // FIXME Check the return value of setVar()
  setVar("mutateeIdle", (void *) &zero, TESTNO, TESTDESC);
  dprintf("%s[%d]:  continue execution for test %d\n", __FILE__, __LINE__, TESTNO);
  appThread->continueExecution();
#endif

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  BPatch_Vector<BPatch_thread *> threads;
  BPatch_process *appProc = appThread->getProcess();
  assert(appProc);
  appProc->getThreads(threads);
  int active_threads = 11; // FIXME Magic number
  threads.clear();
  while (((test3_threadCreateCounter < TEST3_THREADS)
         || (active_threads > 1))
         && (timeout < TIMEOUT)) {
    dprintf("%s[%d]: waiting for completion for test %d, num active threads = %d\n",
            __FILE__, __LINE__, TESTNO, active_threads);
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    if (appThread->isTerminated()) {
       dprintf("%s[%d]:  BAD NEWS:  somehow the process died\n", __FILE__, __LINE__);
       err = 1;
       break;
    }
    bpatch->pollForStatusChange();
    if (appThread->isStopped()) {
       appThread->continueExecution();
    }
    appProc->getThreads(threads);
    active_threads = threads.size();
    threads.clear();
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(TESTNAME, TESTDESC);
    logerror("%s[%d]:  test timed out. got %d/10 events\n", __FILE__, __LINE__, test3_threadCreateCounter);
    logerror("test3_createCounter is %d, expected %d; active threads %d, expected %d\n",
            test3_threadCreateCounter, TEST3_THREADS, active_threads, 1);
    err = 1;
  }

  dprintf("%s[%d]: ending test %d, num active threads = %d\n",
            __FILE__, __LINE__, TESTNO, active_threads);
  dprintf("%s[%d]:  stop execution for test %d\n", __FILE__, __LINE__, TESTNO);
  appThread->stopExecution();

  //   read all tids from the mutatee and verify that we got them all
  unsigned long mutatee_tids[TEST3_THREADS];
  const char *threads_varname = "test4_threads";
  getVar(threads_varname, (void *) mutatee_tids,
         (sizeof(unsigned long) * TEST3_THREADS),
         TESTNO, TESTDESC);

  if (debugPrint) {
    dprintf("%s[%d]:  read following tids for test%d from mutatee\n", __FILE__, __LINE__, TESTNO);

    for (unsigned int i = 0; i < TEST3_THREADS; ++i) {
       dprintf("\t%lu\n", mutatee_tids[i]);
    }
  }

  for (unsigned int i = 0; i < TEST3_THREADS; ++i) {
     bool found = false;
     for (unsigned int j = 0; j < callback_tids.size(); ++j) {
       if (callback_tids[j] == mutatee_tids[i]) {
         found = true;
         break;
       }
     }

    if (!found) {
      FAIL_MES(TESTNAME, TESTDESC);
      logerror("%s[%d]:  could not find record for tid %lu: have these:\n",
             __FILE__, __LINE__, mutatee_tids[i]);
       for (unsigned int j = 0; j < callback_tids.size(); ++j) {
          logerror("%lu\n", callback_tids[j]);
       }
      err = true;
      break;
    }
  }

  dprintf("%s[%d]: removing thread callback\n", __FILE__, __LINE__);
  if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, createcb)) {
    FAIL_MES(TESTNAME, TESTDESC);
    logerror("%s[%d]:  failed to remove thread callback\n",
           __FILE__, __LINE__);
    err = true;
  }

  if (!err)  {
    PASS_MES(TESTNAME, TESTDESC);
    appThread->getProcess()->terminateExecution();
    return PASSED;
  }
  appThread->getProcess()->terminateExecution();
  return FAILED;
}

test_results_t test_thread_3_Mutator::setup(ParameterDict &param) {
  TestMutator::setup(param);
  debugPrint = param["debugPrint"]->getInt();
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  return PASSED;
}
