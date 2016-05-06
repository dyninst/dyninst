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

// $Id: test1_19.C,v 1.1 2008/10/30 19:18:08 legendre Exp $
/*
 * #Name: test1_19
 * #Desc: Mutator Side - oneTimeCode
 * #Dep: 
 * #Notes: This test needs to be able to manipulate control flow
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test1_19_Mutator : public DyninstMutator {

  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_19_factory() 
{
  return new test1_19_Mutator();
}

static void test19_oneTimeCodeCallback(BPatch_thread * /*thread*/,
				void *userData,
				void * /*returnValue*/)
{
   *(int *)userData = 1;
}

//
// Start Test Case #19 - mutator side (oneTimeCode)
//

test_results_t test1_19_Mutator::executeTest() 
{
    // Avoid a race condition in fast & loose mode

    while (!appProc->isStopped())
	{
		BPatch::bpatch->waitForStatusChange();
    }

    appProc->continueExecution();

    if (waitUntilStopped(BPatch::bpatch, appProc, 19, "oneTimeCode") < 0)
	{
      return FAILED;
    }

    BPatch_Vector<BPatch_function *> bpfv;
    const char *fn = "test1_19_call1";

    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0])
	{
      logerror("    Unable to find function %s\n", fn);
      return FAILED;
    }

    BPatch_function *call19_1_func = bpfv[0];

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call19_1Expr(*call19_1_func, nullArgs);
    checkCost(call19_1Expr);

    appProc->oneTimeCode(call19_1Expr);

    // Let the mutatee run to check the result
    appProc->continueExecution();

    // Wait for the next test

    if (waitUntilStopped(BPatch::bpatch, appProc, 19, "oneTimeCode") < 0)
	{
      return FAILED;
    }

    bpfv.clear();
    const char *fn2 = "test1_19_call2";

    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0])
	{
      logerror("    Unable to find function %s\n", fn2);
      return FAILED;
    }

    BPatch_function *call19_2_func = bpfv[0];

    BPatch_funcCallExpr call19_2Expr(*call19_2_func, nullArgs);
    checkCost(call19_2Expr);

    int callbackFlag = 0;

    // Register a callback that will set the flag callbackFlag
    BPatchOneTimeCodeCallback oldCallback = 
       BPatch::bpatch->registerOneTimeCodeCallback(test19_oneTimeCodeCallback);

    appProc->oneTimeCodeAsync(call19_2Expr, (void *)&callbackFlag);

    while (!appProc->isTerminated() && !appProc->isStopped() )
    {
		BPatch::bpatch->waitForStatusChange();
    }
    
    // Continue mutatee after one-time code runs
    appProc->continueExecution();

    // Wait for the callback to be called
    while (!appProc->isTerminated() && !callbackFlag) {
        if( !BPatch::bpatch->waitForStatusChange() ) {
            logerror("   FAILED: could not wait for callback to be called\n");
            return FAILED;
        }
    }

    if( !callbackFlag ) {
        logerror("     FAILED: process %d terminated while waiting for async oneTimeCode\n",
                appProc->getPid());
        return FAILED;
    }

    // After the oneTimeCode is completed, there could be a crash due to bugs in
    // the RPC code, wait for termination
    while( !appProc->isTerminated() ) {
        if( !BPatch::bpatch->waitForStatusChange() ) {
            logerror("   FAILED: could not wait for process to terminate\n");
            return FAILED;
        }
    }

    // Restore old callback (if there was one)
	BPatch::bpatch->registerOneTimeCodeCallback(oldCallback);

    return PASSED;
} // test1_19_Mutator::executeTest()

