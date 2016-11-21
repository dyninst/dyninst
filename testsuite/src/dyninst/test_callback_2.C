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

// $Id: test_callback_2.C,v 1.1 2008/10/30 19:21:26 legendre Exp $
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
#include "BPatch_point.h"

#include "test_lib.h"
#include "test12.h"

#include "dyninst_comp.h"
class test_callback_2_Mutator : public DyninstMutator {
protected:
  BPatch *bpatch;

  void dumpVars();
  bool setVar(const char *vname, void *addr, int testno, const char *testname);
  BPatchSnippetHandle *at(BPatch_point * pt, BPatch_function *call,
			  int testno, const char *testname);

public:
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_callback_2_factory() {
  return new test_callback_2_Mutator();
}

#define TESTNO 7
#define TESTNAME "test_callback_2"
#define TESTDESC "user defined message callback -- st"


bool test7done = false;
bool test7err = false;
unsigned long test7_tids[TEST8_THREADS];
int callback_counter = 0;

std::vector<user_msg_t> elog;

static BPatch_point *findPoint(BPatch_function *f, BPatch_procedureLocation loc,
                        int testno, const char *testname)
{
  assert(f);
  BPatch_Vector<BPatch_point *> *pts = f->findPoint(loc);

  if (!pts) {
	  logerror("%s[%d]:  failed to find point\n", FILE__, __LINE__);
    FAIL_MES(TESTNAME, TESTDESC);
    return NULL;
  }

  if (pts->size() != 1) {
	  logerror("%s[%d]:  failed to find point: found too many\n", FILE__, __LINE__);
      FAIL_MES(TESTNAME, TESTDESC);
      return NULL;
  }

  return (*pts)[0];
}

//  at -- simple instrumentation.  As written, only can insert funcs without args -- 
//     -- modify to take snippet vector args if necessary.
BPatchSnippetHandle *
test_callback_2_Mutator::at(BPatch_point * pt, BPatch_function *call,
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

  if (!ret) 
  {
    logerror("%s[%d]:  could not insert instrumentation\n", __FILE__, __LINE__);
    FAIL_MES(TESTNAME, TESTDESC);
    test7err = true;
  }

  return ret;
}

void test_callback_2_Mutator::dumpVars()
{
  BPatch_Vector<BPatch_variableExpr *> vars;
  appImage->getVariables(vars);
  for (unsigned int i = 0; i < vars.size(); ++i) {
    logerror("\t%s\n", vars[i]->getName());
  }
}

bool test_callback_2_Mutator::setVar(const char *vname, void *addr, int testno,
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


static void test7cb(BPatch_process *  proc, void *buf, unsigned int bufsize)
{
  if (debugPrint())
    dprintf("%s[%d]:  inside test7cb\n", __FILE__, __LINE__);

  if (bufsize != sizeof(user_msg_t)) {
    //  something is incredibly wrong
    logerror("%s[%d]:  unexpected message size %d not %d\n",
            __FILE__, __LINE__, bufsize, sizeof(user_msg_t));
    test7err = true;
    return;
  }

  user_msg_t *msg = (user_msg_t *) buf;
  user_event_t what = msg->what;
  unsigned long tid = msg->tid;

  if (debugPrint())
    dprintf("%s[%d]:  thread = %lu, what = %d\n", __FILE__, __LINE__, tid, what);

  elog.push_back(*msg);

  if (proc->getPid() != tid)
  {
	  fprintf(stderr, "%s[%d]:  ERROR:  got event for pid %lu, not %d\n", FILE__, __LINE__, tid, proc->getPid());
  }

  if (callback_counter == 0) {
    //  we expect the entry point to be reported first
    if (what != func_entry) {
      logerror("%s[%d]:  unexpected message %d not %d\n",
            __FILE__, __LINE__, (int) what, (int) func_entry);
      FAIL_MES(TESTNAME, TESTDESC);
      test7err = true;
      return;
    }
  } else if (callback_counter <= TEST7_NUMCALLS) {
    // we expect to get a bunch of function calls next
    if (what != func_callsite) {
      logerror("%s[%d]:  unexpected message %d not %d\n",
            __FILE__, __LINE__, (int) what, (int) func_callsite);
      FAIL_MES(TESTNAME, TESTDESC);
      test7err = true;
      return;
    }
  }
  else if (callback_counter == (TEST7_NUMCALLS +1)) {
    // lastly comes the function exit
    if (what != func_exit) {
      logerror("%s[%d]:  unexpected message %d not %d\n",
            __FILE__, __LINE__, (int) what, (int) func_exit);
      FAIL_MES(TESTNAME, TESTDESC);
      test7err = true;
      return;
    }
    // set test7done to end the test
    test7done = true;
  }
  callback_counter++;
}

void log_res()
{
	logerror("%s[%d]:  Here's what happened: \n", FILE__, __LINE__);
	for (unsigned int i = 0; i < elog.size(); ++i)
	{
		std::string ewhat;
		switch (elog[i].what) {
			case func_entry: ewhat = std::string("func_entry"); break;
			case func_callsite: ewhat = std::string("func_callsite"); break;
			case func_exit: ewhat = std::string("func_exit"); break;
			default:
							ewhat = std::string("unknown_event"); break;
		}
		logerror("\t %s:  %d\n", ewhat.c_str(), elog[i].tid);
  }
}

// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test_callback_2_Mutator::executeTest() 
{
	//  a simple single threaded user messagin scenario where we want to send
	//  async messages at function entry/exit and call points.

	// load libtest12.so -- currently only used by subtest 5, but make it generally
	// available
	const char *libname = "./libTest12.so";    
	test7err = false;
	test7done = false;
	callback_counter = 0;
	elog.resize(0);

#if defined(arch_x86_64_test)
	if (appProc->getAddressWidth() == 4)
		libname = "./libTest12_m32.so";
#endif

	dprintf("%s[%d]:  loading test library: %s\n", __FILE__, __LINE__, libname);

	if (!appProc->loadLibrary(libname))
	{
		logerror("%s[%d]:  failed to load library %s, cannot proceed\n", 
				__FILE__, __LINE__, libname);
                appProc->terminateExecution();
		return FAILED;
	}

	dprintf("%s[%d]:  loaded test library: %s\n", __FILE__, __LINE__, libname);
	BPatchUserEventCallback cb = test7cb;

	if (!bpatch->registerUserEventCallback(cb)) 
	{
		FAIL_MES(TESTNAME, TESTDESC);
		logerror("%s[%d]: could not register callback\n", __FILE__, __LINE__);
                appProc->terminateExecution();
		return FAILED;
	}

	//  instrument entry and exit of call7_1, as well as call points inside call7_1

	const char *call1name = "test_callback_2_call1";

	BPatch_function *call7_1 = findFunction(call1name, appImage,TESTNO, TESTNAME);

	BPatch_point *entry = findPoint(call7_1, BPatch_entry,TESTNO, TESTNAME);

	if (NULL == entry) 
	{
		logerror("%s[%d]: Unable to find entry point to function %s\n", 
				__FILE__, __LINE__, call1name);
		bpatch->removeUserEventCallback(test7cb);
                appProc->terminateExecution();
		return FAILED;
	}

	BPatch_point *exit = findPoint(call7_1, BPatch_exit,TESTNO, TESTNAME);

	if (NULL == entry) 
	{
		logerror("%s[%d]:  Unable to find exit point to function %s\n", 
				__FILE__, __LINE__, call1name);
		bpatch->removeUserEventCallback(test7cb);
                appProc->terminateExecution();
		return FAILED;
	}

	BPatch_point *callsite = findPoint(call7_1, BPatch_subroutine,TESTNO, TESTNAME);

	if (NULL == callsite) 
	{
		logerror("%s[%d]:  Unable to find subroutine call point in function %s\n",
				__FILE__, __LINE__, call1name);
		bpatch->removeUserEventCallback(test7cb);
                appProc->terminateExecution();
		return FAILED;
	}

	//  These are our asynchronous message functions (in libTest12) that we
	//  attach to the "interesting" points

	BPatch_function *reportEntry = findFunction("reportEntry", appImage,TESTNO, TESTNAME);
	BPatch_function *reportExit = findFunction("reportExit", appImage,TESTNO, TESTNAME);
	BPatch_function *reportCallsite = findFunction("reportCallsite", appImage,TESTNO, TESTNAME);

	if (!reportEntry)
	{
		logerror("%s[%d]:  reportEntry = NULL\n", FILE__, __LINE__);
		bpatch->removeUserEventCallback(test7cb);
                appProc->terminateExecution();
		return FAILED;
	}
	if (!reportExit)
	{
		logerror("%s[%d]:  reportExit = NULL\n", FILE__, __LINE__);
		bpatch->removeUserEventCallback(test7cb);
                appProc->terminateExecution();
		return FAILED;
	}
	if (!reportCallsite)
	{
		logerror("%s[%d]:  reportCallsite = NULL\n", FILE__, __LINE__);
		bpatch->removeUserEventCallback(test7cb);
                appProc->terminateExecution();
		return FAILED;
	}

	//  Do the instrumentation
	BPatchSnippetHandle *entryHandle = at(entry, reportEntry, TESTNO, TESTNAME);
	BPatchSnippetHandle *exitHandle = at(exit, reportExit, TESTNO, TESTNAME);
	BPatchSnippetHandle *callsiteHandle = at(callsite, reportCallsite, TESTNO, TESTNAME);

	//  "at" may set test7err
	if ((test7err)
			||  (NULL == entryHandle) 
			|| (NULL == exitHandle) 
			|| (NULL == callsiteHandle) )
	{
		logerror("%s[%d]:  instrumentation failed, test7err = %d\n", FILE__, __LINE__, test7err);
		logerror("%s[%d]:  entryHandle = %p\n", FILE__, __LINE__, entryHandle);
		logerror("%s[%d]:  exitHandle = %p\n", FILE__, __LINE__, exitHandle);
		logerror("%s[%d]:  callsiteHandle = %p\n", FILE__, __LINE__, callsiteHandle);
		bpatch->removeUserEventCallback(test7cb);
		return FAILED;
	}
	if (debugPrint()) 
	{
		int one = 1;
		const char *varName = "libraryDebug";
		if (setVar(varName, (void *) &one, TESTNO, TESTNAME)) 
		{
			logerror("%s[%d]:  Error setting variable '%s' in mutatee\n", 
					FILE__, __LINE__, varName);
			bpatch->removeUserEventCallback(test7cb);
                        appProc->terminateExecution();
			return FAILED;
		}
	}

	dprintf("%s[%d]:  did instrumentation, continuing process...: %s\n", 
			__FILE__, __LINE__, libname);
	//  unset mutateeIdle to trigger mutatee to issue messages.

	int timeout = 0;

        appProc->continueExecution();

	dprintf("%s[%d]:  continued process...: %s\n", 
			__FILE__, __LINE__, libname);
	//  wait until we have received the desired number of events
	//  (or timeout happens)

	while(!test7err && !test7done && (timeout < TIMEOUT)) 
	{
		sleep_ms(SLEEP_INTERVAL/*ms*/);
		timeout += SLEEP_INTERVAL;
		bpatch->pollForStatusChange();

                if (appProc->isTerminated())
		{
			BPatch_exitType et = appProc->terminationStatus();
			if (et == ExitedNormally)
			{
				int ecode = appProc->getExitCode();
				logerror("%s[%d]:  normal exit with code %d\n",
						__FILE__, __LINE__, ecode);
			}
			if (et == ExitedViaSignal)
			{
				int ecode = appProc->getExitSignal();
				logerror("%s[%d]:  caught signal %d\n",
						__FILE__, __LINE__, ecode);
			}
			log_res();
			bpatch->removeUserEventCallback(test7cb);
			return FAILED;
		}
	}

	dprintf("%s[%d]:  after wait loop:  test7err = %s, test7done = %s, timeout = %d\n", 
			__FILE__, __LINE__, test7err ? "true" : "false", test7done ? "true" : "false", timeout);

	if (timeout >= TIMEOUT) 
	{
		FAIL_MES(TESTNAME, TESTDESC);
		logerror("%s[%d]:  test timed out: %d ms\n",
				__FILE__, __LINE__, TIMEOUT);
		test7err = true;
	}

	if (!appProc->stopExecution())
	{
		logerror("%s[%d]:  stopExecution failed\n",
				__FILE__, __LINE__);

	}

	dprintf("%s[%d]:  stopped process...\n", 
			__FILE__, __LINE__ );

	if (!bpatch->removeUserEventCallback(test7cb)) 
	{
		FAIL_MES(TESTNAME, TESTDESC);
		logerror("%s[%d]:  failed to remove callback\n",
				__FILE__, __LINE__);
		appProc->terminateExecution();
		log_res();
		return FAILED;
	}

	dprintf("%s[%d]:  removed callback...\n", 
			__FILE__, __LINE__ );
	if (!appProc->terminateExecution())
	{
		//  don't care
		//fprintf(stderr, "%s[%d]:  terminateExecution failed\n", FILE__, __LINE__);
		//return FAILED;
	}

#if 0
	int one = 1;

	if (setVar("test_callback_2_idle", (void *) &one, TESTNO, TESTNAME)) 
	{
		logerror("Error setting variable 'test_callback_2_idle' in mutatee\n");
		test7err = true;
		appProc->terminateExecution();
	}
#endif

#if 0
	// And let it run out
	if (!appThread->isTerminated()) 
	{
		appProc->continueExecution();
	}

	while (!appThread->isTerminated()) 
	{
		// Workaround for pgCC_high mutatee issue
		bpatch->waitForStatusChange();
	}
#endif

	if (!test7err) 
	{
		PASS_MES(TESTNAME, TESTDESC);
		return PASSED;
	}

	log_res();
	FAIL_MES(TESTNAME, TESTDESC);
	return FAILED;
}

// extern "C" int test12_7_mutatorMAIN(ParameterDict &param)
test_results_t test_callback_2_Mutator::setup(ParameterDict &param) {
	DyninstMutator::setup(param);
	bpatch = (BPatch *)(param["bpatch"]->getPtr());
	return PASSED;
}
