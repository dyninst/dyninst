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

// $Id: test4_1.C,v 1.1 2008/10/30 19:20:47 legendre Exp $
/*
 * #Name: test4_1
 * #Desc: Exit Callback
 * #Dep: 
 * #Arch: !(i386_unknown_nt4_0_test,alpha_dec_osf4_0_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#define MAX_TEST	4

#include "dyninst_comp.h"
class test4_1_Mutator : public DyninstMutator {
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test4_1_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
  test_results_t mutatorTest();
};
extern "C" DLLEXPORT  TestMutator *test4_1_factory() {
  return new test4_1_Mutator();
}

test4_1_Mutator::test4_1_Mutator()
  : bpatch(NULL), pathname(NULL) {
}

static bool passedTest = false;
static BPatch_process *myprocs[25];
static int threadCount = 0;

static void forkFunc(BPatch_thread *parent, BPatch_thread *child)
{
    dprintf("forkFunc called with parent %p, child %p\n", parent, child);
    if (child) myprocs[threadCount++] = child->getProcess();

    if (!child) {
        dprintf("in prefork for %d\n", parent->getProcess()->getPid());
    } else {
        dprintf("in fork of %d to %d\n", parent->getProcess()->getPid(), child->getProcess()->getPid());
    }
}

static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type)
{
  dprintf("exitFunc called\n");
    // Read out the values of the variables.
  if(thread->getProcess()->terminationStatus() != exit_type) 
  {
    passedTest = false;
    return;
    
  }
  int exitCode = thread->getProcess()->getExitCode();
  int expectedExitCode = thread->getProcess()->getPid();
#if !defined(os_windows_test)
  expectedExitCode &= 0xFF;
#endif
  
  // Read out the values of the variables.
  if (exit_type == ExitedNormally) {
    if(expectedExitCode == exitCode) {
      if (verifyChildMemory(thread->getProcess(), "test4_1_global1", 1000001)) {
	logerror("Passed test #1 (exit callback)\n");
	passedTest = true;
      } else {
	logerror("**Failed** test #1 (exit callback)\n");
	logerror("    verifyChildMemory failed\n");
	passedTest = false;
      }
    } else {
      logerror("**Failed** test #1 (exit callback)\n");
      logerror("    exit code = %d, was not equal to expected %d\n", exitCode, 
	       expectedExitCode);
      passedTest = false;
    }
  } else if (exit_type == ExitedViaSignal) {
    logerror("**Failed** test #1 (exit callback), exited via signal %d\n",
	     thread->getProcess()->getExitSignal());
    passedTest = false;
  } else assert(false);
}

static void execFunc(BPatch_thread *thread)
{
    logerror("**Failed Test #1\n");
    logerror("    execCallback invoked, but exec was not called!\n");
}

test_results_t test4_1_Mutator::mutatorTest() {
    int n = 0;
    const char *child_argv[MAX_TEST+7];
	
    dprintf("in mutatorTest1\n");

    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test4_1");
    child_argv[n] = NULL;

    // Start the mutatee
    logerror("Starting \"%s\"\n", pathname);

    appProc = bpatch->processCreate(pathname, child_argv,NULL);
    dprintf("Test 1: using thread %p\n", appProc);
    if (appProc == NULL) {
	logerror("Unable to run test program.\n");
        return FAILED;
    }
    contAndWaitForAllProcs(bpatch, appProc, myprocs, &threadCount);

    if ( !passedTest )
    {
        logerror("**Failed** test #1 (exit callback)\n");
        logerror("    exit callback not executed\n");
        return FAILED;
    }

    return PASSED;
}

test_results_t test4_1_Mutator::executeTest() {
  // Register the proper callbacks for this test
  bpatch->registerPreForkCallback(forkFunc);
  bpatch->registerPostForkCallback(forkFunc);
  bpatch->registerExecCallback(execFunc);
  bpatch->registerExitCallback(exitFunc);

  test_results_t rv = mutatorTest();

  // Remove callbacks upon test completion
  bpatch->registerPreForkCallback(NULL);
  bpatch->registerPostForkCallback(NULL);
  bpatch->registerExecCallback(NULL);
  bpatch->registerExitCallback(NULL);

  return rv;
}

// extern "C" TEST_DLL_EXPORT int test4_1_mutatorMAIN(ParameterDict &param)
test_results_t test4_1_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    return PASSED;
}
