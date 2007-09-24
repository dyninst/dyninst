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

// $Id: test_thread_4.C,v 1.1 2007/09/24 16:41:38 cooksey Exp $
/*
 * #Name: test12_5
 * #Desc: thread exit callback 
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
class test_thread_4_Mutator : public TestMutator {
protected:
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
extern "C" TEST_DLL_EXPORT TestMutator *test_thread_4_factory() {
  return new test_thread_4_Mutator();
}

#define TESTNO 5
#define TESTNAME "thread exit callback"

static int debugPrint;

void test_thread_4_Mutator::dumpVars(void)
{
  BPatch_Vector<BPatch_variableExpr *> vars;
  appImage->getVariables(vars);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    logerror("\t%s\n", vars[i]->getName());
  }
}

// Returns false on success and true on error
bool test_thread_4_Mutator::setVar(const char *vname, void *addr, int testno, const char *testname)
{
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

// Returns false on success and true on error
bool test_thread_4_Mutator::getVar(const char *vname, void *addr, int len, int testno, const char *testname)
{
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

static int test5_threadDestroyCounter = 0;
static void threadDestroyCB(BPatch_process * /*proc*/, BPatch_thread *thr)
{
  if (debugPrint) 
    dprintf("%s[%d]:  thread %lu destroy event for pid %d\n",
            __FILE__, __LINE__, thr->getTid(), thr->getPid());
  test5_threadDestroyCounter++;
}  
      
// static bool mutatorTest5and6(int testno, const char *testname)
test_results_t test_thread_4_Mutator::execute() {     
  unsigned int timeout = 0; // in ms
  int err = 0;

  BPatchAsyncThreadEventCallback destroycb = threadDestroyCB;
  if (!bpatch->registerThreadEventCallback(BPatch_threadDestroyEvent, destroycb))
  {   
    FAIL_MES(testno, testname);
    logerror("%s[%d]:  failed to register thread callback\n",
           __FILE__, __LINE__);
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }



   if (debugPrint)
    dprintf("%s[%d]:  registered threadDestroy callback\n",
            __FILE__, __LINE__);
      
  //  unset mutateeIdle to trigger thread (10) spawn.
         
  int zero = 0;
  setVar("mutateeIdle", (void *) &zero, testno, testname);
  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)
  while(test5_threadDestroyCounter < TEST5_THREADS && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    dprintf("%s[%d]:  polling\n", __FILE__, __LINE__);
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(testno, testname);
    logerror("%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    err = 1;
  }

  appThread->stopExecution();

  if (!bpatch->removeThreadEventCallback(BPatch_threadDestroyEvent, destroycb)) {
    FAIL_MES(testno, testname);
    logerror("%s[%d]:  failed to remove thread callback\n",
           __FILE__, __LINE__);
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }

  if (!err) {
    PASS_MES(testno, testname);
    appThread->getProcess()->terminateExecution();
    return PASSED;
  }
  appThread->getProcess()->terminateExecution();
  return PASSED;
}


static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
  // os_none?
#if defined (os_none)
  if (mutatorTest5and6(TESTNO, TESTNAME))
    return 0;
  return -1;
#else
  SKIP(TESTNO, TESTNAME);
  return 0;
#endif
}

// extern "C" int test12_5_mutatorMAIN(ParameterDict &param)
test_results_t test_thread_4_Mutator::setup(ParameterDict &param) {
  return SKIPPED; // This test appears to be disabled for all platforms..

  TestMutator::setup(param);
  debugPrint = param["debugPrint"]->getInt();
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  return PASSED;
}
