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

// $Id: test_callback_1.C,v 1.1 2007/09/24 16:40:25 cooksey Exp $
/*
 * #Name: test12_2
 * #Desc: dynamic callsite callback
 * #Dep: 
 * #Arch: !(os_windows,os_irix)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test12.h"

#include "TestMutator.h"
class test_callback_1_Mutator : public TestMutator {
protected:
  BPatch *bpatch;

public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
};
extern "C" TEST_DLL_EXPORT TestMutator *test_callback_1_factory() {
  return new test_callback_1_Mutator();
}

#define TESTNO 2
#define TESTNAME "test_callback_1"
#define TESTDESC "dynamic callsite callback"

static const char *expected_fnames[] = {"call2_1","call2_2","call2_3","call2_4"};
static int test2done = 0;
static int test2err = 0;
static int mutateeXLC = 0;
static int debugPrint;
template class BPatch_Vector<void *>;
static BPatch_Vector<BPatch_point *> test2handles;
static BPatch_Vector<BPatch_point *> dyncalls;
static BPatch_thread *globalThread = NULL;
extern BPatch *bpatch;
  static int counter = 0;
  static int counter2 = 0;

static void dynSiteCB(BPatch_point *dyn_site, BPatch_function *called_function)
{
  //dprintf("%s[%d]:  dynSiteCB: pt = %p. func = %p.\n",
  //                 __FILE__, __LINE__, dyn_site, called_function);
  //  static int counter = 0;
  //  static int counter2 = 0;
  BPatch_point *pt = dyn_site;
  BPatch_function *func = called_function;
  assert(pt);
  assert(func);

  void *callsite_addr = pt->getAddress();
  dprintf("%s[%d]:  callsite addr = %p\n", __FILE__, __LINE__, callsite_addr);

  char buf[2048];
  func->getName(buf, 2048);
  //dprintf("%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
  //        expected_fnames[counter]);
  if (strcmp(expected_fnames[counter], buf)) {
    FAIL_MES(TESTNAME, TESTDESC);
    dprintf("\t%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
          expected_fnames[counter]);
    globalThread->stopExecution();
    test2done = 1;
  }
  counter++;
  if (counter > 3) {
    counter = 0;
    counter2++;
  }

  if (counter2 >= 2) {
    bool removal_error = false;
    globalThread->stopExecution();
    //  not passed yet, now remove dynamic call monitoring handles
    assert (test2handles.size());
    for (unsigned int i = 0; i < test2handles.size(); ++i) {
      if (!test2handles[i]->stopMonitoring()) {
        removal_error = true;
      }
    }
    if (removal_error) {
      FAIL_MES("test_callback_1", TESTDESC);
      test2err = 1;
    }else {
      PASS_MES("test_callback_1", TESTDESC);
    }
    test2done = 1;
  }
}


// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test_callback_1_Mutator::execute() {
  dprintf("%s[%d]:  welcome to test12_2\n", __FILE__, __LINE__);
  int timeout = 0;
  globalThread = appThread;
  test2done = 0;
  test2err = 0;
  dyncalls.clear();
  test2handles.clear();
  counter = 0;
  counter2 = 0;

  if (mutateeXLC) {
    // Why are we continuing the mutatee?  Doesn't it just start looping
    // forever when we do this?
    //appThread->continueExecution();
     appThread->terminateExecution();
     SKIP(TESTNAME, TESTDESC);
     logerror("\txlc optimizes out dynamic call sites for this test\n");
     sleep_ms(100);
     return SKIPPED;
  }

  if (!bpatch->registerDynamicCallCallback(dynSiteCB)) {
     FAIL_MES(TESTNAME, TESTDESC);
     logerror("  failed to register callsite callback\n");
     appThread->terminateExecution();
     return FAILED;
  }

  BPatch_function *func2_1 = findFunction("call2_dispatch", appThread->getImage(), TESTNO, TESTNAME);
  BPatch_function *targetFunc = func2_1;

  BPatch_Vector<BPatch_point *> *calls = targetFunc->findPoint(BPatch_subroutine);
  if (!calls) {
     FAIL_MES(TESTNAME, TESTDESC);
     logerror("  cannot find call points for func1_1\n");
     appThread->terminateExecution();
     return FAILED;
  }

  for (unsigned int i = 0; i < calls->size(); ++i) {
    BPatch_point *pt = (*calls)[i];
    if (pt->isDynamic()){
      bool ret;
      ret = pt->monitorCalls();
      if (!ret) {
        FAIL_MES(TESTNAME, TESTDESC);
        logerror("  failed monitorCalls\n");
	appThread->terminateExecution();
	return FAILED;
      }
      test2handles.push_back(pt);
      dyncalls.push_back(pt);
    }
  }

  if (dyncalls.size() != 3) {
     FAIL_MES(TESTNAME, TESTDESC);
     logerror("  wrong number of dynamic points found (%d -- not 3)\n",
             dyncalls.size());
     logerror("  total number of calls found: %d\n", calls->size());
     appThread->terminateExecution();
     return FAILED;
  }

  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  while(!test2done && (timeout < TIMEOUT)) {
    bpatch->pollForStatusChange();
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(TESTNAME, TESTDESC);
    logerror("%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test2err = 1;
  }

  if (test2err) {
    appThread->terminateExecution();
    return FAILED;
  } else {
    appThread->terminateExecution();
    return PASSED;
  }
}

//extern "C" int test12_2_mutatorMAIN(ParameterDict &param)
test_results_t test_callback_1_Mutator::setup(ParameterDict &param) {
#ifdef os_windows
  return SKIPPED;
#else
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();
    mutateeXLC = param["mutateeXLC"]->getInt();

    if (TestMutator::setup(param) == FAILED) {
      return FAILED;
    }
    
    // TODO Move the sanity check block below into execute()

    //  sanity checks?? -- should not be necesssary, but if they're not mentioned here, they aren't always found
    //  later, suggests issue with delayed parsing?
    BPatch_function *f;
    if (NULL == (f = findFunction("call2_1", appImage, TESTNO, TESTNAME)))
      return FAILED;
    if (NULL == (f = findFunction("call2_2", appImage, TESTNO, TESTNAME)))
      return FAILED;
    if (NULL == (f = findFunction("call2_3", appImage, TESTNO, TESTNAME)))
      return FAILED;
    if (NULL == (f = findFunction("call2_4", appImage, TESTNO, TESTNAME)))
      return FAILED;

    return PASSED;
#endif // Not Windows
}
