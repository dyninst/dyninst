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

// $Id: test4_1.C,v 1.3 2008/06/18 19:58:26 carl Exp $
/*
 * #Name: test4_1
 * #Desc: Exit Callback
 * #Dep: 
 * #Arch: !(i386_unknown_nt4_0,alpha_dec_osf4_0)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#define MAX_TEST	4

#include "TestMutator.h"
class test4_1_Mutator : public TestMutator {
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test4_1_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
  test_results_t mutatorTest();
};
extern "C" TEST_DLL_EXPORT TestMutator *test4_1_factory() {
  return new test4_1_Mutator();
}

test4_1_Mutator::test4_1_Mutator()
  : bpatch(NULL), pathname(NULL) {
}

static bool passedTest = false;
static BPatch_thread *mythreads[25];
static int threadCount = 0;

static void forkFunc(BPatch_thread *parent, BPatch_thread *child)
{
  dprintf("forkFunc called with parent %p, child %p\n", parent, child);
    BPatch_image *appImage;
    BPatch_Vector<BPatch_function *> bpfv;
    BPatch_Vector<BPatch_snippet *> nullArgs;

    if (child) mythreads[threadCount++] = child;

    if (!child) {
       dprintf("in prefork for %d\n", parent->getPid());
    } else {
       dprintf("in fork of %d to %d\n", parent->getPid(), child->getPid());
    }
}

static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type)
{
  dprintf("exitFunc called\n");
    // Read out the values of the variables.
    int exitCode = thread->getExitCode();

    assert(thread->terminationStatus() == exit_type);
    // Read out the values of the variables.
    if (exit_type == ExitedNormally) {
        if(thread->getPid() == exitCode) {
            if (verifyChildMemory(thread, "test4_1_global1", 1000001)) {
                logerror("Passed test #1 (exit callback)\n");
                passedTest = true;
            } else {
                logerror("**Failed** test #1 (exit callback)\n");
                logerror("    verifyChildMemory failed\n");
                passedTest = false;
            }
        } else {
            logerror("**Failed** test #1 (exit callback)\n");
            logerror("    exit code = %d, was not equal to pid\n", exitCode);
            passedTest = false;
        }
    } else if (exit_type == ExitedViaSignal) {
       logerror("**Failed** test #1 (exit callback), exited via signal %d\n",
               thread->getExitSignal());
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
    if (getPIDFilename() != NULL) {
      child_argv[n++] = const_cast<char *>("-pidfile");
      child_argv[n++] = getPIDFilename();
    }
    child_argv[n] = NULL;

    // Start the mutatee
    logerror("Starting \"%s\"\n", pathname);

    BPatch_thread *appThread = bpatch->createProcess(pathname, child_argv,NULL);
    dprintf("Test 1: using thread %p\n", appThread);
    if (appThread == NULL) {
	logerror("Unable to run test program.\n");
        return FAILED;
    }
    // Register for cleanup
    registerPID(appThread->getProcess()->getPid());

    contAndWaitForAllThreads(bpatch, appThread, mythreads, &threadCount);

    if ( !passedTest )
    {
        logerror("**Failed** test #1 (exit callback)\n");
        logerror("    exit callback not executed\n");
        return FAILED;
    }

    return PASSED;
}

test_results_t test4_1_Mutator::execute() {
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
