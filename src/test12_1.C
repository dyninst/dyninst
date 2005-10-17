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

// $Id: test12_1.C,v 1.2 2005/10/17 19:14:27 bpellin Exp $
/*
 * #Name: test12_1
 * #Desc: dynamic call site callback
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
#define TESTNAME "dynamic call site callback"

int mutateeXLC;
BPatch *bpatch;
int debugPrint;

BPatch_thread *appThread;
static const char *expected_fnames[] = {"call1_1","call1_2","call1_3","call1_4"};
int test1done = 0;
int test1err = 0;
template class BPatch_Vector<void *>;
BPatch_Vector<void *> test1handles;
BPatch_Vector<BPatch_point *> dyncalls;

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
  if (debugPrint) 
    fprintf(stderr, "%s[%d]:  callsite addr = %p\n", __FILE__, __LINE__, callsite_addr);
  char buf[2048];
  func->getName(buf, 2048);
  //fprintf(stderr, "%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
  //        expected_fnames[counter]);
  if (strcmp(expected_fnames[counter], buf)) {
    FAIL_MES(TESTNO, TESTNAME);
    printf("\t%s[%d]:  got func %s, expect func %s\n", __FILE__, __LINE__, buf,
          expected_fnames[counter]);
    appThread->stopExecution();      
    test1done = 1;
  }
  counter++;
  if (counter > 3) {
    counter = 0;
    counter2++;
  }

  if (counter2 >= 2) {
    bool removal_error = false;
    appThread->stopExecution();      
    //  not passed yet, now remove dynamic call monitoring handles
    assert (test1handles.size());
    for (unsigned int i = 0; i < test1handles.size(); ++i) {
      if (!dyncalls[i]->removeDynamicCallCallback(test1handles[i])) {
        removal_error = true;
      }
    }
    if (removal_error) {
      FAIL_MES(TESTNO, TESTNAME);
      test1err = 1;
    }else {
      PASS_MES(TESTNO, TESTNAME);
    }
    test1done = 1;
  }
}

int mutatorTest(BPatch_thread *appT, BPatch_image *appImage)
{
  int timeout = 0;
  appThread = appT;

  if (mutateeXLC) {
     appThread->continueExecution();
     SKIP(TESTNO, TESTNAME);
     fprintf(stderr, "\txlc optimizes out dynamic call sites for this test\n");
     sleep_ms(100);
     return 0;
  }

  BPatch_function *func1_1 = findFunction("call1_dispatch", appImage, TESTNO, TESTNAME);
  if ( func1_1 == NULL ) return -1;
  BPatch_function *targetFunc = func1_1;

  BPatch_Vector<BPatch_point *> *calls = targetFunc->findPoint(BPatch_subroutine);
  if (!calls) {
     FAIL_MES(TESTNO, TESTNAME);
     fprintf(stderr, "  cannot find call points for func1_1\n");
     exit(1);
  }

  for (unsigned int i = 0; i < calls->size(); ++i) {
    BPatch_point *pt = (*calls)[i];
    if (pt->isDynamic()){
      void *handle = NULL;
      handle = pt->registerDynamicCallCallback(dynSiteCB);
      if (!handle) {
        FAIL_MES(TESTNO, TESTNAME);
        fprintf(stderr, "  registerDynamicCallCallback failed\n");
        exit(1);
      } 
      test1handles.push_back(handle);
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

  appThread->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)

  while(!test1done && (timeout < TIMEOUT)) {
    bpatch->pollForStatusChange();
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test1err = 1;
  }

  return (test1err == 0);
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

    // Signal the child that we've attached
    if (useAttach) {
        signalAttached(appThread, appImage);
    }

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
