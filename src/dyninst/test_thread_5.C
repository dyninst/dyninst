/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: 
/*
 * #Name: test12_8
 * #Desc: user defined message callback -- mt
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
#include "BPatch_point.h"
#include "BPatch_object.h"

#include "test_lib.h"
#include "test12.h"

#include "dyninst_comp.h"
class test_thread_5_Mutator : public DyninstMutator {
protected:
  BPatch *bpatch;

  BPatchSnippetHandle *at(BPatch_point * pt, BPatch_function *call,
			  int testno, const char *testname);
  void dumpVars();
  bool setVar(const char *vname, void *addr, int testno,
              const char *testname);



public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_thread_5_factory() {
  return new test_thread_5_Mutator();
}

#define TESTNO 8
#define TESTNAME "test_thread_5"
#define TESTDESC "user defined message callback -- mt"


static BPatch_point *findPoint(BPatch_function *f, BPatch_procedureLocation loc,
                        int testno, const char *testname)
{
  assert(f);
  BPatch_Vector<BPatch_point *> *pts = f->findPoint(loc);

  if (!pts) {
    FAIL_MES(testno, testname);
    logerror("%s[%d]:  no points matching requested location\n", __FILE__, __LINE__);
    return NULL;
  }

  if (pts->size() != 1) {
    FAIL_MES(testno, testname);
    logerror("%s[%d]:  %d points matching requested location, not 1\n", __FILE__, __LINE__,
           pts->size());
    return NULL;
  }

  return (*pts)[0];
}

//  at -- simple instrumentation.  As written, only can insert funcs without args -- 
//     -- modify to take snippet vector args if necessary.
BPatchSnippetHandle *
test_thread_5_Mutator::at(BPatch_point * pt, BPatch_function *call,
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
  ret = appProc->insertSnippet(snip, *pt,when);

  if (!ret) {
    FAIL_MES(testno, testname);
    logerror("%s[%d]:  could not insert instrumentation\n", __FILE__, __LINE__);
    return NULL;
  }

  return ret;
}

void test_thread_5_Mutator::dumpVars() {
  BPatch_Vector<BPatch_variableExpr *> vars;
  appImage->getVariables(vars);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    logerror("\t%s\n", vars[i]->getName());
  }
}

// Returns false on success and true on error
bool test_thread_5_Mutator::setVar(const char *vname, void *addr, int testno,
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

static bool test8done = false;
static bool test8err = false;
static unsigned long tids[TEST8_THREADS];
static user_event_t last_event[TEST8_THREADS];

static bool findThreadIndex(unsigned long tid, unsigned int &index)
{
 //  find the index with tid <tid>, if it exists, otherwise, the index of
 //  an empty slot.  If no empty slot, return false (fail);
  for (index = 0; index < TEST8_THREADS; ++index) {
    if (0 == tids[index]) {
      tids[index] = tid;
      if (debugPrint())
        dprintf("%s[%d]:  giving new slot to thread id %lu\n",
                __FILE__, __LINE__, tid);
      return true;
    }
    if (tid == tids[index])
      return true;
  }
  return false;
}

static void test8cb(BPatch_process * /*proc*/, void *buf, unsigned int bufsize)
{
  // FIXME This being static is probably killing the test
  static int destroy_counter = 0;
  if (debugPrint())
    dprintf("%s[%d]:  inside test8cb\n", __FILE__, __LINE__);

  if (bufsize != sizeof(user_msg_t)) {
    //  something is incredibly wrong
    logerror("%s[%d]:  unexpected message size %d not %d\n",
            __FILE__, __LINE__, bufsize, sizeof(user_msg_t));
    test8err = true;
    return;
  }

  user_msg_t *msg = (user_msg_t *) buf;
  user_event_t what = msg->what;
  unsigned long tid = msg->tid;

  if (debugPrint())
    dprintf("%s[%d]:  thread = %lu, what = %d\n", __FILE__, __LINE__, tid, what);
  unsigned int index;
  if (!findThreadIndex(tid, index)) {
    test8err = true;
    logerror("%s[%d]:  failed to find record for tid %lu (or empty slot)\n",
            __FILE__, __LINE__,tid);
    return;
  }

  if (debugPrint())
    dprintf("%s[%d]:  thread id %lu: index %d\n", __FILE__, __LINE__, tid, index);
  if (last_event[index] != (what - 1)) {
    test8err = true;
    logerror("%s[%d]:  out of order messsage received for thread %lu, last = %d, now = %d\n",
           __FILE__, __LINE__, tid, last_event[index], what);
    return;
  }
  last_event[index] = what;

  if (what == mutex_destroy) {
     destroy_counter++;
  }
  if (destroy_counter == TEST8_THREADS) {
    destroy_counter = 0;
    test8done = true;
  }
  //sleep_ms(10);
}

// Macro to do some return code checking after calls to findPoint()
#define CHECK_POINT(pt, ptname, fnname) \
do { \
  if (NULL == (pt)) { \
    logerror("**Failed test_thread_5\n"); \
    logerror("    Unable to find %s point to %s\n", (ptname), (fnname)); \
    appThread->getProcess()->terminateExecution(); \
    return FAILED; \
  } \
} while (0)

#define CHECK_AT(snippet) \
do { \
  if (NULL == snippet) { \
    logerror("**Failed test_thread_5\n"); \
    logerror("    Failed to insert snippet\n"); \
    appThread->getProcess()->terminateExecution(); \
    return FAILED; \
  } \
} while (0)

#if defined(os_freebsd_test)
const char *threadLibName = "libthr";
#else
const char *threadLibName = "libpthread";
#endif

// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test_thread_5_Mutator::executeTest() {
  test8done = false;
  test8err = false;

    // load libtest12.so -- currently only used by subtest 5, but make it generally
    // available
    const char *libname = "./libTest12.so";    
#if defined(arch_x86_64_test)
    if (appThread->getProcess()->getAddressWidth() == 4)
       libname = "./libTest12_m32.so";
#endif
    dprintf("%s[%d]:  loading test library: %s\n", __FILE__, __LINE__, libname);
    if (!appProc->loadLibrary(libname)) {
      logerror("TERMINATE: %s[%d]:  failed to load library %s, cannot proceed\n", 
	      __FILE__, __LINE__, libname);
      appThread->getProcess()->terminateExecution();
      return FAILED;
    }

  for (unsigned int i = 0; i < TEST8_THREADS; ++i) {
    tids[i] = 0;
    last_event[i] = null_event;
  }

  //  instrument events having to do with mutex init, lock, unlock, destroy
  //  with messaging functions in libtest12.so
  BPatch_object *libpthread = NULL;
  vector<BPatch_object*> objs;
  appImage->getObjects(objs);
  for(auto i = objs.begin(); i != objs.end(); ++i)
  {
      if((*i)->name().find(threadLibName) != std::string::npos)
      {
	  libpthread = *i;
      }
  }
  assert(libpthread);

  BPatch_function *mutInit = findFunction("createLock", appImage,TESTNO, TESTDESC);
  BPatch_point *mutInitPt = findPoint(mutInit, BPatch_entry,TESTNO, TESTDESC);
  CHECK_POINT(mutInitPt, "entry", "createLock");
  BPatch_function *reportInit = findFunction("reportMutexInit", appImage,TESTNO, TESTDESC);
  BPatchSnippetHandle *initHandle = at(mutInitPt, reportInit, TESTNO, TESTDESC);
  CHECK_AT(initHandle);
  BPatch_function *mutDestroy = findFunction("destroyLock", appImage,TESTNO, TESTDESC);
  BPatch_point *mutDestroyPt = findPoint(mutDestroy, BPatch_entry,TESTNO, TESTDESC);
  CHECK_POINT(mutDestroyPt, "entry", "destroyLock");
  BPatch_function *reportDestroy = findFunction("reportMutexDestroy", appImage,TESTNO, TESTDESC);
  BPatchSnippetHandle *destroyHandle = at(mutDestroyPt, reportDestroy, TESTNO, TESTDESC);
  CHECK_AT(destroyHandle);

  BPatch_function *mutLock = findFunction("lockLock", appImage,TESTNO, TESTDESC);
  BPatch_point *mutLockPt = findPoint(mutLock, BPatch_entry,TESTNO,TESTDESC);
  CHECK_POINT(mutLockPt, "entry", "lockLock");
  BPatch_function *reportLock = findFunction("reportMutexLock", appImage,TESTNO, TESTDESC);
  BPatchSnippetHandle *lockHandle = at(mutLockPt, reportLock, TESTNO, TESTDESC);
  CHECK_AT(lockHandle);
  BPatch_function *mutUnlock = findFunction("unlockLock", appImage,TESTNO, TESTDESC);
  BPatch_point *mutUnlockPt = findPoint(mutUnlock, BPatch_entry,TESTNO,TESTDESC);
  CHECK_POINT(mutUnlockPt, "entry", "unlockLock");
  BPatch_function *reportUnlock = findFunction("reportMutexUnlock", appImage,TESTNO, TESTDESC);
  BPatchSnippetHandle *unlockHandle = at(mutUnlockPt, reportUnlock, TESTNO, TESTDESC);
  CHECK_AT(unlockHandle);

  BPatchUserEventCallback cb = test8cb;
  if (!bpatch->registerUserEventCallback(cb)) {
    FAIL_MES(TESTNAME, TESTDESC);
    logerror("TERMINATE: %s[%d]: could not register callback\n", __FILE__, __LINE__);
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }


  int timeout = 0;
  appThread->getProcess()->continueExecution();

  //  wait until we have received the desired number of events
  while(!test8err && !test8done) {
    bpatch->waitForStatusChange();
  }

  if (timeout >= TIMEOUT) {
    FAIL_MES(TESTNAME, TESTDESC);
    logerror("%s[%d]:  test timed out. Took longer than %d ms\n",
           __FILE__, __LINE__, TIMEOUT);
    test8err = true;
  }

  appThread->getProcess()->stopExecution();

  int one = 1;
  // I need to check the return value for this function call
  logerror("TERMINATE: setting exit variable\n");
  if (setVar("test_thread_5_idle", (void *) &one, TESTNO, TESTDESC)) {
    logerror("TERMINATE: Unable to set variable test_thread_5_idle\n");
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }

  if (!bpatch->removeUserEventCallback(test8cb)) {
    FAIL_MES(TESTNAME, TESTDESC);
    logerror("TERMINATE: %s[%d]:  failed to remove callback\n",
           __FILE__, __LINE__);
    appThread->getProcess()->terminateExecution();
    return FAILED;
  }

  //  what happens here if we still have messages coming in, but no handler -- should
  //  check this?


  appThread->getProcess()->deleteSnippet(initHandle);
  appThread->getProcess()->deleteSnippet(destroyHandle);
  appThread->getProcess()->deleteSnippet(lockHandle);
  appThread->getProcess()->deleteSnippet(unlockHandle);

  appThread->getProcess()->continueExecution();

  while (!appThread->getProcess()->isTerminated()) {
    bpatch->waitForStatusChange();
  }

  if (!test8err) {
    PASS_MES(TESTNAME, TESTDESC);
    return PASSED;
  }
  return FAILED;
}

#undef CHECK_POINT // Clean up after myself
#undef CHECK_AT

// extern "C" int test12_8_mutatorMAIN(ParameterDict &param)
test_results_t test_thread_5_Mutator::setup(ParameterDict &param) {
  DyninstMutator::setup(param);
  bpatch = (BPatch *)(param["bpatch"]->getPtr());
  return PASSED;
}
