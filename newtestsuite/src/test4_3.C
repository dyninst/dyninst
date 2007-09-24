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

// $Id: test4_3.C,v 1.1 2007/09/24 16:39:53 cooksey Exp $
/*
 * #Name: test4_3
 * #Desc: Exec Callback
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
class test4_3_Mutator : public TestMutator {
  const unsigned int MAX_TEST;
  BPatch *bpatch;
  char *pathname;

public:
  test4_3_Mutator();
  virtual bool hasCustomExecutionPath() { return true; }
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t execute();
  virtual test_results_t mutatorTest();
};
extern "C" TEST_DLL_EXPORT TestMutator *test4_3_factory() {
  return new test4_3_Mutator();
}

test4_3_Mutator::test4_3_Mutator()
  : MAX_TEST(4), bpatch(NULL), pathname(NULL) {
}

static bool passedTest = false;
static int threadCount = 0;
static BPatch_thread *mythreads[25];
static int debugPrint;

static void forkFunc(BPatch_thread *parent, BPatch_thread *child)
{
  // I think this test should set failure in the fork callback.  The test4_3
  // mutatee doesn't call fork..
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

    assert(thread->terminationStatus() == exit_type);
    // Read out the values of the variables.

    // simple exec 
    if(exit_type == ExitedViaSignal) {
        logerror("Failed test #3 (exec callback), exited via signal %d\n",
               thread->getExitSignal());
    } else if (!verifyChildMemory(thread, "globalVariable3_1", 3000002)) {
        logerror("Failed test #3 (exec callback)\n");
    } else {
        logerror("Passed test #3 (exec callback)\n");
        passedTest = true;
    }
}

static void execFunc(BPatch_thread *thread)
{
        BPatch_Vector<BPatch_function *> bpfv;
	dprintf("in exec callback for %d\n", thread->getPid());

	// insert code into parent
	BPatch_Vector<BPatch_snippet *> nullArgs;
        BPatch_image *appImage = thread->getImage();
        assert(appImage);

	char *fn = "test4_3_func2";
	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  logerror("    Unable to find function %s\n",fn);
	  // FIXME Remove the call to exit(). This should return an error
	  // instead
	  exit(1);
	}

        BPatch_function *func3_2_parent = bpfv[0];
        BPatch_funcCallExpr callExpr(*func3_2_parent, nullArgs);

	bpfv.clear();
	char *fn2 = "test4_3_func1";
	if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	    || NULL == bpfv[0]){
	  logerror("    Unable to find function %s\n",fn2);
	  // FIXME Remove the call to exit(). This should return an error
	  // instead
	  exit(1);
	}

	BPatch_function *func3_1_parent = bpfv[0];
	BPatch_Vector<BPatch_point *> *point = func3_1_parent->findPoint(BPatch_exit);
	
	// So we're inserting a call to func2 at the end of func1
	// We're making sure that we keep track of what's going on over the
	// course of the exec() and that the snippet gets inserted into the
	// correct process / address space.

        assert(point);
        thread->insertSnippet(callExpr, *point);
	dprintf("%s[%d]:  MUTATEE: exec callback for %d, done with insert snippet\n", __FILE__, __LINE__, thread->getPid());
}

test_results_t test4_3_Mutator::mutatorTest() {
#if defined(i386_unknown_nt4_0)
    logerror("Skipping test #3 (exec callback)\n");
    logerror("    not implemented on this platform\n");
    return SKIPPED;
#else

    int n = 0;
    const char *child_argv[MAX_TEST+5];
	
    dprintf("in mutatorTest3\n");

    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    child_argv[n++] = const_cast<char*>("-run");
    child_argv[n++] = const_cast<char*>("test4_3");
    child_argv[n] = NULL;

    // Start the mutatee
    logerror("Starting \"%s\"\n", pathname);

    BPatch_thread *appThread = bpatch->createProcess(pathname, child_argv,NULL);
    if (appThread == NULL) {
	logerror("Unable to run test program.\n");
        return FAILED;
    }

    contAndWaitForAllThreads(bpatch, appThread, mythreads, &threadCount);

    if ( !passedTest )
    {
        logerror("**Failed** test #3 (exec callback)\n");
        logerror("    exec callback not executed\n");
        return FAILED;
    }
    return PASSED;
#endif
}

test_results_t test4_3_Mutator::execute() {
  passedTest = false;
  threadCount = 0;

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

// extern "C" TEST_DLL_EXPORT int test4_3_mutatorMAIN(ParameterDict &param)
test_results_t test4_3_Mutator::setup(ParameterDict &param) {
    pathname = param["pathname"]->getString();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    debugPrint = param["debugPrint"]->getInt();

    return PASSED;
}
