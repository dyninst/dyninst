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

// $Id: test4_2.C,v 1.2 2008/05/08 20:54:39 cooksey Exp $
/*char *
 * #Name: test4_2
 * #Desc: Fork Callback
 * #Dep: 
 * #Arch: !(i386_unknown_nt4_0,alpha_dec_osf4_0)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "TestMutator.h"
class test4_2_Mutator : public TestMutator {
  const unsigned int MAX_TEST;
  int debugPrint;
  BPatch *bpatch;
  char *pathname;

public:
  test4_2_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
  test_results_t mutatorTest();
};
extern "C" TestMutator *test4_2_factory() {
  return new test4_2_Mutator();
}

test4_2_Mutator::test4_2_Mutator()
  : MAX_TEST(4), bpatch(NULL), pathname(NULL) {
}

static bool passedTest;
static int threadCount;
static BPatch_thread *mythreads[25];
static BPatch_thread *test2Child;
static BPatch_thread *test2Parent;
static int exited;

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

    if (!child) return;	// skip prefork case

    // Make a race condition always show up -- we don't run
    // until the processes have had a chance.
#if !defined(os_windows)
    sleep(1);
#endif
    // That'll make erroneous continues break...

    // insert code into parent
    appImage = parent->getImage();
    assert(appImage);

    char *fn = "test4_2_func3";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn);
         exit(1);
    }

    BPatch_function *func3_parent = bpfv[0];
    BPatch_funcCallExpr callExpr2(*func3_parent, nullArgs);
 
    bpfv.clear();
    char *fn2 = "test4_2_func2";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn2);
         exit(1);
    }

    BPatch_function *func2_parent = bpfv[0];
    BPatch_Vector<BPatch_point *> *point2 = func2_parent->findPoint(BPatch_exit);
    assert(point2);

    parent->insertSnippet(callExpr2, *point2);

    dprintf("MUTATEE:  after insert in fork of %d to %d\n", parent->getPid(), child->getPid());
    // insert different code into child
    appImage = child->getImage();
    assert(appImage);

    bpfv.clear();
    char *fn3 = "test4_2_func4";
    if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn3);
         exit(1);
    }

    BPatch_function *func4_child = bpfv[0];
    BPatch_funcCallExpr callExpr1(*func4_child, nullArgs);

    bpfv.clear();
    char *fn4 = "test4_2_func2";
    if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
	   || NULL == bpfv[0]){
	 logerror("    Unable to find function %s\n",fn4);
         exit(1);
    }

    BPatch_function *func2_child = bpfv[0];
    BPatch_Vector<BPatch_point *> *point1 = func2_child->findPoint(BPatch_exit);
    assert(point1);

    child->insertSnippet(callExpr1, *point1);

    dprintf("MUTATEE:  after insert2 in fork of %d to %d\n", parent->getPid(), child->getPid());
    test2Child = child;
    test2Parent = parent;
}

static void exitFunc(BPatch_thread *thread, BPatch_exitType exit_type)
{
  dprintf("exitFunc called\n");
  bool failedTest = false;
    // Read out the values of the variables.
    int exitCode = thread->getExitCode();

    assert(thread->terminationStatus() == exit_type);
    // Read out the values of the variables.
    exited++;
    if(exit_type == ExitedViaSignal) {
        logerror("Failed test #2 (fork callback)\n");
        logerror("    a process terminated via signal %d\n",
               thread->getExitSignal());
        exited = 0;
    } else if (thread->getPid() != exitCode) {
        logerror("Failed test #2 (fork callback)\n");
        logerror("    exit code was not equal to pid (%d != %d)\n",
		 thread->getPid(), exitCode);
        exited = 0;
    } else {
        dprintf("test #2, pid %d exited\n", exitCode);
        if ((test2Parent == thread) &&
            !verifyChildMemory(test2Parent, "test4_2_global1", 2000002)) {
            failedTest = true;
        }
        if ((test2Child == thread) &&
            !verifyChildMemory(test2Child, "test4_2_global1", 2000003)) {
            failedTest = true;
        }

        // See if all the processes are done
        if (exited == 2) {
            if (!failedTest) {
                logerror("Passed test #2 (fork callback)\n");
                passedTest = true;
            } else {
                logerror("Failed test #2 (fork callback)\n");
            }
	    // exited = 0; // For the next time through..
        }
    }
}

static void execFunc(BPatch_thread *thread)
{
    logerror("**Failed Test #2\n");
    logerror("    execCallback invoked, but exec was not called!\n");
}

test_results_t test4_2_Mutator::mutatorTest() {
#if defined(i386_unknown_nt4_0)
    logerror("Skipping test #2 (fork callback)\n");
    logerror("    not implemented on this platform\n");
    return SKIPPED;
#else

    int n = 0;
    const char *child_argv[MAX_TEST+7];

    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test4_2");
    /* TODO I'd like to automate this part, or wrap it somehow.. */
    if (getPIDFilename() != NULL) {
      child_argv[n++] = const_cast<char *>("-pidfile");
      child_argv[n++] = getPIDFilename();
    }
    child_argv[n] = NULL;

    // Start the mutatee
    logerror("Starting \"%s\"\n", pathname);

    BPatch_thread *appThread = bpatch->createProcess(pathname, child_argv,NULL);
    dprintf("Process %p created", appThread);
    if (appThread == NULL) {
	logerror("Unable to run test program.\n");
        return FAILED;
    }
    // Register for cleanup
    registerPID(appThread->getProcess()->getPid());

    contAndWaitForAllThreads(bpatch, appThread, mythreads, &threadCount);

    if ( !passedTest )
    {
        logerror("**Failed** test #2 (fork callback)\n");
	// FIXME This error message is misleading
        logerror("    fork callback not executed\n");
        return FAILED;
    }
    return PASSED;

#endif
}

test_results_t test4_2_Mutator::execute() {
  passedTest = false;
  threadCount = 0;
  exited = 0;

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

// extern "C" TEST_DLL_EXPORT int test4_2_mutatorMAIN(ParameterDict &param)
test_results_t test4_2_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    return PASSED;
}
