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

// $Id: test12_7.C,v 1.2 2006/06/11 00:35:35 legendre Exp $
/*
 * #Name: test12_7
 * #Desc: user defined message callback -- st
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

#define TESTNO 7
#define TESTNAME "user defined message callback -- st"

int debugPrint;
BPatch *bpatch;
BPatch_thread *appThread;
BPatch_image *appImage;

BPatch_point *findPoint(BPatch_function *f, BPatch_procedureLocation loc,
                        int testno, const char *testname)
{
  assert(f);
  BPatch_Vector<BPatch_point *> *pts = f->findPoint(loc);

  if (!pts) {
    FAIL_MES(testno, testname);
    fprintf(stderr, "%s[%d]:  no points matching requested location\n", __FILE__, __LINE__);
    exit(1);
  }

  if (pts->size() != 1) {
    FAIL_MES(testno, testname);
    fprintf(stderr, "%s[%d]:  %d points matching requested location, not 1\n", __FILE__, __LINE__,
           pts->size());
    exit(1);
  }

  return (*pts)[0];
}

//  at -- simple instrumentation.  As written, only can insert funcs without args -- 
//     -- modify to take snippet vector args if necessary.
BPatchSnippetHandle *at(BPatch_point * pt, BPatch_function *call,
                        int testno, const char *testname)
{
  BPatch_Vector<BPatch_snippet *> args;
  BPatch_funcCallExpr snip(*call, args);
  BPatch_procedureLocation pttype = pt->getPointType();
  BPatch_callWhen when;
  if (pttype == BPatch_entry) when = BPatch_callBefore;
  else if (pttype == BPatch_exit) when = BPatch_callAfter;
  else if (pttype == BPatch_subroutine) when = BPatch_callBefore;
  else assert(0);

  BPatchSnippetHandle *ret;
  ret = appThread->insertSnippet(snip, *pt,when);

  if (!ret) {
    FAIL_MES(testno, testname);
    fprintf(stderr, "%s[%d]:  could not insert instrumentation\n", __FILE__, __LINE__);
    exit(1);
  }

  return ret;
}

void dumpVars(void)
{
  BPatch_Vector<BPatch_variableExpr *> vars;
  appImage->getVariables(vars);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    fprintf(stderr, "\t%s\n", vars[i]->getName());
  }
}


void setVar(const char *vname, void *addr, int testno, const char *testname)
{
   BPatch_variableExpr *v;
   void *buf = addr;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  cannot find variable %s, avail vars:\n", vname);
      dumpVars();
         exit(1);
   }

   if (! v->writeValue(buf, sizeof(int),true)) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  failed to write call site var to mutatee\n");
      exit(1);
   }
}

void getVar(const char *vname, void *addr, int len, int testno, const char *testname)
{
   BPatch_variableExpr *v;
   if (NULL == (v = appImage->findVariable(vname))) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  cannot find variable %s: avail vars:\n", vname);
      dumpVars();
         exit(1);
   }

   if (! v->readValue(addr, len)) {
      fprintf(stderr, "**Failed test #%d (%s)\n", testno, testname);
      fprintf(stderr, "  failed to read var in mutatee\n");
      exit(1);
   }
}

bool test7done = false;
bool test7err = false;
unsigned long test7_tids[TEST8_THREADS];

void test7cb(BPatch_process * /* proc*/, void *buf, unsigned int bufsize)
{
  static int callback_counter = 0;
  if (debugPrint)
    fprintf(stderr, "%s[%d]:  inside test7cb\n", __FILE__, __LINE__);

  if (bufsize != sizeof(user_msg_t)) {
    //  something is incredibly wrong
    fprintf(stderr, "%s[%d]:  unexpected message size %d not %d\n",
            __FILE__, __LINE__, bufsize, sizeof(user_msg_t));
    test7err = true;
    return;
  }

  user_msg_t *msg = (user_msg_t *) buf;
  user_event_t what = msg->what;
  unsigned long tid = msg->tid;

  if (debugPrint)
    fprintf(stderr, "%s[%d]:  thread = %lu, what = %d\n", __FILE__, __LINE__, tid, what);


  if (callback_counter == 0) {
    //  we expect the entry point to be reported first
    if (what != func_entry) {
      fprintf(stderr, "%s[%d]:  unexpected message %d not %d\n",
            __FILE__, __LINE__, (int) what, (int) func_entry);
      FAIL_MES(TESTNO, TESTNAME);
      test7err = 1;
      return;
    }
  } else if (callback_counter <= TEST7_NUMCALLS) {
    // we expect to get a bunch of function calls next
    if (what != func_callsite) {
      fprintf(stderr, "%s[%d]:  unexpected message %d not %d\n",
            __FILE__, __LINE__, (int) what, (int) func_callsite);
      FAIL_MES(TESTNO, TESTNAME);
      test7err = 1;
      return;
    }
  }
  else if (callback_counter == (TEST7_NUMCALLS +1)) {
    // lastly comes the function exit
    if (what != func_exit) {
      fprintf(stderr, "%s[%d]:  unexpected message %d not %d\n",
            __FILE__, __LINE__, (int) what, (int) func_exit);
      FAIL_MES(TESTNO, TESTNAME);
      test7err = 1;
      return;
    }
    // set test7done to end the test
    test7done = true;
  }
  callback_counter++;
}

int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
  //  a simple single threaded user messagin scenario where we want to send
  //  async messages at function entry/exit and call points.

    // load libtest12.so -- currently only used by subtest 5, but make it generally
    // available
    char *libname = "./libTest12.so";    
#if defined(arch_x86_64)
    if (appThread->getProcess()->getAddressWidth() == 4)
      libname = "./libTest12_m32.so";
#endif
    dprintf("%s[%d]:  loading test library: %s\n", __FILE__, __LINE__, libname);
    if (!appThread->loadLibrary(libname)) {
      fprintf(stderr, "%s[%d]:  failed to load library %s, cannot proceed\n", 
	      __FILE__, __LINE__, libname);
      return -1;
    }

  BPatchUserEventCallback cb = test7cb;
  if (!bpatch->registerUserEventCallback(cb)) {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]: could not register callback\n", __FILE__, __LINE__);
    return -1;
  }

  //  instrument entry and exit of call7_1, as well as call points inside call7_1
  BPatch_function *call7_1 = findFunction("call7_1", appImage,TESTNO, TESTNAME);
  BPatch_point *entry = findPoint(call7_1, BPatch_entry,TESTNO, TESTNAME);
  BPatch_point *exit = findPoint(call7_1, BPatch_exit,TESTNO, TESTNAME);
  BPatch_point *callsite = findPoint(call7_1, BPatch_subroutine,TESTNO, TESTNAME);

  //  These are our asynchronous message functions (in libTest12) that we
  //  attach to the "interesting" points
  BPatch_function *reportEntry = findFunction("reportEntry", appImage,TESTNO, TESTNAME);
  BPatch_function *reportExit = findFunction("reportExit", appImage,TESTNO, TESTNAME);
  BPatch_function *reportCallsite = findFunction("reportCallsite", appImage,TESTNO, TESTNAME);

  //  Do the instrumentation
  BPatchSnippetHandle *entryHandle = at(entry, reportEntry, TESTNO, TESTNAME);
  BPatchSnippetHandle *exitHandle = at(exit, reportExit, TESTNO, TESTNAME);
  BPatchSnippetHandle *callsiteHandle = at(callsite, reportCallsite, TESTNO, TESTNAME);


  if (debugPrint) {
     int one = 1;
     setVar("libraryDebug", (void *) &one, TESTNO, TESTNAME);
  }
 //  unset mutateeIdle to trigger mutatee to issue messages.

  int zero = 0;
  int timeout = 0;
  setVar("mutateeIdle", (void *) &zero, TESTNO, TESTNAME);
  appThread->getProcess()->continueExecution();

  //  wait until we have received the desired number of events
  //  (or timeout happens)
  while(!test7err && !test7done && (timeout < TIMEOUT)) {
    sleep_ms(SLEEP_INTERVAL/*ms*/);
    timeout += SLEEP_INTERVAL;
    bpatch->pollForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  test timed out.\n",
           __FILE__, __LINE__);
    test7err = true;
  }

  appThread->getProcess()->stopExecution();

  if (!bpatch->removeUserEventCallback(test7cb)) {
    FAIL_MES(TESTNO, TESTNAME);
    fprintf(stderr, "%s[%d]:  failed to remove callback\n",
           __FILE__, __LINE__);
    return -1;
  }

  if (!test7err) {
    PASS_MES(TESTNO, TESTNAME);
    return 0;
  }

  FAIL_MES(TESTNO, TESTNAME);
  return -1;
}

extern "C" int mutatorMAIN(ParameterDict &param)
{
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
        signalAttached(appThread, appImage);
    }

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
