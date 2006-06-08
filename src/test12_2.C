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

// $Id: test12_2.C,v 1.4 2006/06/08 12:25:13 jaw Exp $
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

#define TESTNO 2
#define TESTNAME "dynamic callsite callback"

static const char *expected_fnames[] = {"call2_1","call2_2","call2_3","call2_4"};
int test2done = 0;
int test2err = 0;
int mutateeXLC = 0;
int debugPrint;
template class BPatch_Vector<void *>;
BPatch_Vector<BPatch_point *> test2handles;
BPatch_Vector<BPatch_point *> dyncalls;
BPatch_thread *globalThread = NULL;
extern BPatch *bpatch;

void dynSiteCB(BPatch_point *dyn_site, BPatch_function *called_function)
{
  //fprintf(stderr, "%s[%d]:  dynSiteCB: pt = %p. func = %p.\n",
  //                 __FILE__, __LINE__, dyn_site, called_function);
  static int counter = 0;
  static int counter2 = 0;
  BPatch_point *pt = dyn_site;
  BPatch_function *func = called_function;
  assert(pt);
  assert(func);

  void *callsite_addr = pt->getAddress();
  dprintf("%s[%d]:  callsite addr = %p\n", __FILE__, __LINE__, callsite_addr);

  char buf[2048];
  func->getName(buf, 2048);
  //fprintf(stderr, "%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
  //        expected_fnames[counter]);
  if (strcmp(expected_fnames[counter], buf)) {
    FAIL_MES(TESTNO, TESTNAME);
    printf("\t%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
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
      FAIL_MES(TESTNO, TESTNAME);
      test2err = 1;
    }else {
      PASS_MES(TESTNO, TESTNAME);
    }
    test2done = 1;
  }
}


int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
#if !defined (os_windows) 
  dprintf("%s[%d]:  welcome to test12_2\n", __FILE__, __LINE__);
  int timeout = 0;
  globalThread = appThread;

  if (mutateeXLC) {
     appThread->continueExecution();
     SKIP(TESTNO, TESTNAME);
     fprintf(stderr, "\txlc optimizes out dynamic call sites for this test\n");
     sleep_ms(100);
     return true;
  }

  if (!bpatch->registerDynamicCallCallback(dynSiteCB)) {
     FAIL_MES(TESTNO, TESTNAME);
     fprintf(stderr, "  failed to register callsite callback\n");
     exit(1);
  }

  BPatch_function *func2_1 = findFunction("call2_dispatch", appThread->getImage(), TESTNO, TESTNAME);
  BPatch_function *targetFunc = func2_1;

  BPatch_Vector<BPatch_point *> *calls = targetFunc->findPoint(BPatch_subroutine);
  if (!calls) {
     FAIL_MES(TESTNO, TESTNAME);
     fprintf(stderr, "  cannot find call points for func1_1\n");
     exit(1);
  }


  for (unsigned int i = 0; i < calls->size(); ++i) {
    BPatch_point *pt = (*calls)[i];
    if (pt->isDynamic()){
      bool ret;
      ret = pt->monitorCalls();
      if (!ret) {
        FAIL_MES(TESTNO, TESTNAME);
        fprintf(stderr, "  failed monitorCalls\n");
        exit(1);
      }
      test2handles.push_back(pt);
      dyncalls.push_back(pt);
    }
  }

  if (dyncalls.size() != 3) {
     FAIL_MES(TESTNO, TESTNAME);
     fprintf(stderr, "  wrong number of dynamic points found (%d -- not 3)\n",
             dyncalls.size());
     fprintf(stderr, "  total number of calls found: %d\n", calls->size());
        exit(1);
  }

  fprintf(stderr, "%s[%d]:  before continue execution\n", __FILE__, __LINE__);
  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  while(!test2done && (timeout < TIMEOUT)) {
    bpatch->pollForStatusChange();
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test2err = 1;
  }

  return (test2err == 0);

#else
    SKIP_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  This test is not supported on this platform\n",
                    __FILE__, __LINE__);
#endif
  return 0;
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();
    mutateeXLC = param["mutateeXLC"]->getInt();


    
    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    //  sanity checks?? -- should not be necesssary, but if they're not mentioned here, they aren't always found
    //  later, suggests issue with delayed parsing?
    BPatch_function *f;
    if (NULL == (f = findFunction("call2_1", appImage, TESTNO, TESTNAME))) abort();
    if (NULL == (f = findFunction("call2_2", appImage, TESTNO, TESTNAME))) abort();
    if (NULL == (f = findFunction("call2_3", appImage, TESTNO, TESTNAME))) abort();
    if (NULL == (f = findFunction("call2_4", appImage, TESTNO, TESTNAME))) abort();

    // Signal the child that we've attached
    if (useAttach) {
        signalAttached(appThread, appImage);
    }

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
